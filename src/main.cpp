#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>
#include <wx/filename.h>
#include <wx/clipbrd.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>
#include <wx/webrequest.h>
#ifdef __WXMSW__
#include <wx/msw/wrapwin.h>
#endif

#include "Frame_Base.h"

namespace
{
constexpr int kServerStatusRequestId = wxID_HIGHEST + 101;
constexpr int kServerTimeRequestId = wxID_HIGHEST + 102;
constexpr int kCheckoutRequestId = wxID_HIGHEST + 103;
constexpr int kNewOrderRequestId = wxID_HIGHEST + 104;
constexpr int kSkuLookupRequestId = wxID_HIGHEST + 105;
constexpr char kServerStatusUrl[] = "http://127.0.0.1:8868/";
constexpr char kServerTimeUrl[] = "http://127.0.0.1:8868/time";
constexpr char kCheckoutUrl[] = "http://127.0.0.1:8868/api/order/check_out_payment_batch";
constexpr char kNewOrderUrl[] = "http://127.0.0.1:8868/api/order/caritxnbaru";
constexpr char kSkuLookupUrl[] = "http://127.0.0.1:8868/api/produk/cek_sku";

std::string EscapeJsonString(const wxString& value)
{
    const wxScopedCharBuffer utf8 = value.utf8_str();
    const std::string input(utf8.data(), utf8.length());
    std::string escaped;
    escaped.reserve(input.size());

    for (const unsigned char ch : input)
    {
        switch (ch)
        {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\b':
            escaped += "\\b";
            break;
        case '\f':
            escaped += "\\f";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            if (ch < 0x20)
            {
                static const char hex[] = "0123456789abcdef";
                escaped += "\\u00";
                escaped += hex[ch >> 4];
                escaped += hex[ch & 0x0f];
            }
            else
            {
                escaped += static_cast<char>(ch);
            }
        }
    }

    return escaped;
}

bool JsonSuccessIsTrue(const wxString& response)
{
    const std::string json = response.ToStdString();
    const std::size_t key = json.find("\"success\"");
    if (key == std::string::npos)
    {
        return false;
    }

    const std::size_t colon = json.find(':', key + 9);
    if (colon == std::string::npos)
    {
        return false;
    }

    std::size_t value = colon + 1;
    while (value < json.size() && std::isspace(static_cast<unsigned char>(json[value])))
    {
        ++value;
    }

    return json.compare(value, 4, "true") == 0 &&
           (value + 4 == json.size() ||
            json[value + 4] == ',' ||
            json[value + 4] == '}' ||
            std::isspace(static_cast<unsigned char>(json[value + 4])));
}

bool TryGetJsonString(const wxString& response, const char* key, wxString& value)
{
    const std::string json = response.ToStdString();
    const std::string quotedKey = std::string("\"") + key + "\"";
    const std::size_t keyPosition = json.find(quotedKey);
    if (keyPosition == std::string::npos)
    {
        return false;
    }

    const std::size_t colon = json.find(':', keyPosition + quotedKey.length());
    const std::size_t openingQuote = colon == std::string::npos
        ? std::string::npos
        : json.find('"', colon + 1);
    if (openingQuote == std::string::npos)
    {
        return false;
    }

    const std::size_t closingQuote = json.find('"', openingQuote + 1);
    if (closingQuote == std::string::npos)
    {
        return false;
    }

    value = wxString::FromUTF8(
        json.substr(openingQuote + 1, closingQuote - openingQuote - 1));
    return !value.empty();
}

bool TryGetJsonNumber(const wxString& response, const char* key, double& value)
{
    const std::string json = response.ToStdString();
    const std::string quotedKey = std::string("\"") + key + "\"";
    const std::size_t keyPosition = json.find(quotedKey);
    if (keyPosition == std::string::npos)
    {
        return false;
    }
    const std::size_t colon = json.find(':', keyPosition + quotedKey.length());
    if (colon == std::string::npos)
    {
        return false;
    }
    const char* start = json.c_str() + colon + 1;
    while (*start != '\0' && std::isspace(static_cast<unsigned char>(*start)))
    {
        ++start;
    }
    char* end = nullptr;
    const double parsed = std::strtod(start, &end);
    if (end == start || !std::isfinite(parsed))
    {
        return false;
    }
    while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)))
    {
        ++end;
    }
    if (*end != ',' && *end != '}')
    {
        return false;
    }
    value = parsed;
    return true;
}

bool TryParseRupiah(const wxString& input, long long& amount)
{
    std::string digits;
    const std::string text = input.ToStdString();
    bool decimalSeparatorSeen = false;
    std::string fractionalPart;
    for (const unsigned char ch : text)
    {
        if (std::isspace(ch))
        {
            continue;
        }
        if (ch == '.')
        {
            continue;
        }
        if (ch == ',')
        {
            if (decimalSeparatorSeen)
            {
                return false;
            }
            decimalSeparatorSeen = true;
            continue;
        }
        if (ch < '0' || ch > '9')
        {
            return false;
        }
        (decimalSeparatorSeen ? fractionalPart : digits) += static_cast<char>(ch);
    }

    if (digits.empty() ||
        std::any_of(fractionalPart.begin(), fractionalPart.end(),
                    [](char digit) { return digit != '0'; }))
    {
        return false;
    }

    char* end = nullptr;
    amount = std::strtoll(digits.c_str(), &end, 10);
    return end != digits.c_str() && *end == '\0' && amount >= 0;
}

}

class ClientFrame final : public Frame
{
public:
    ClientFrame()
        : Frame(nullptr)
    {
        Bind(wxEVT_WEBREQUEST_STATE, &ClientFrame::OnWebRequestResponse, this);
        m_buttonTambahItem->SetBitmap(
            wxArtProvider::GetBitmap(wxART_PLUS, wxART_BUTTON, wxSize(16, 16)),
            wxLEFT);
        m_buttonTambahItem->SetBitmapMargins(3, 0);
        m_buttonTambahItem->SetMinSize(wxSize(112, -1));

        m_buttonTambahItem->Bind(wxEVT_BUTTON, &ClientFrame::OnAddItem, this);
        m_btnServerSts->Bind(wxEVT_BUTTON, &ClientFrame::OnServerStatus, this);
        m_btnSvrTime->Bind(wxEVT_BUTTON, &ClientFrame::OnServerTime, this);
        m_btnKeluarKonf->Bind(wxEVT_BUTTON, &ClientFrame::OnExit, this);
        m_buttonCheckOut->Bind(wxEVT_BUTTON, &ClientFrame::OnCheckout, this);
        m_btnItemBatalAll->Bind(wxEVT_BUTTON, &ClientFrame::OnClearItems, this);
        m_btnTxnBatal->Bind(wxEVT_BUTTON, &ClientFrame::OnCancelTransaction, this);
        m_radioBox1->Bind(wxEVT_RADIOBOX, &ClientFrame::OnOrderTypeChanged, this);
        m_textSKU->Bind(wxEVT_TEXT, &ClientFrame::OnSkuChanged, this);
        m_textSKU->Bind(wxEVT_TEXT_ENTER, &ClientFrame::OnSkuEnter, this);
        m_textSKU->Bind(wxEVT_KILL_FOCUS, &ClientFrame::OnSkuFocusLost, this);
        m_spinCtrl1->Bind(wxEVT_SPINCTRL, &ClientFrame::OnQuantityChanged, this);
        m_spinCtrl1->Bind(wxEVT_TEXT, &ClientFrame::OnQuantityTextChanged, this);

        Bind(wxEVT_CHAR_HOOK, &ClientFrame::OnCharHook, this);

        const int statusWidths[] = {-1, 45, 55, 45};
        m_statusBar1->SetFieldsCount(4, statusWidths);
        m_statusBar1->SetStatusText("Aplikasi siap.", 0);
        m_statusBar1->SetStatusText("INS", 1);
        m_statusBar1->SetStatusText("caps", 2);
        m_statusBar1->SetStatusText("---", 3);
        AddItemListColumns();
        UpdateShippingControls();
        UpdateCapsIndicator();
        UpdateClipboardIndicator();
        m_textNoTelp->SetHint("Nomor telepon penerima");
        m_textNamaKurir->SetHint("Nama kurir, misalnya SPX");
        m_textTipeSumber->SetHint("Nama E-Commerce, misalnya SHOPEE");
        m_textTipeSumber->SetToolTip(
            "Rekomendasi Kiriman yang didukung merchant kami adalah "
            "SHOPEE, TOKOPEDIA, BLI-BLI, BUKALAPAK, LAZADA, INSTAGRAM, "
            "TIKTOK, FACEBOOK, WHATSAPP. Merchant lainnya dapat diisi manual.");
        m_textPenerima->Clear();
        m_textAlamat->Clear();
        m_textNoTelp->Clear();
        m_textNamaKurir->Clear();
        m_textTipeSumber->Clear();
        m_textOngkir->ChangeValue("0,00");
        m_textNewTxn->Enable(true);
        m_textNewTxn->SetEditable(false);
        m_textNewTxn->Clear();
        m_textNewTxn->SetToolTip("Nomor transaksi diberikan oleh server.");
        m_textSKU->Enable(true);
        m_spinCtrl1->Enable(true);
        m_buttonTambahItem->Disable();
        m_buttonCheckOut->Disable();
        m_statusBar1->SetStatusText("Memeriksa nomor transaksi dari server...", 0);
        ReserveNewTransaction(AfterNewTransaction::Startup);

#ifdef __WXMSW__
        m_clipboardListenerRegistered = AddClipboardFormatListener(GetHandle()) != FALSE;
        if (!m_clipboardListenerRegistered)
        {
            wxLogSysError("Could not listen for Windows clipboard changes");
        }
#endif
    }

    ~ClientFrame() override
    {
#ifdef __WXMSW__
        if (m_clipboardListenerRegistered)
        {
            RemoveClipboardFormatListener(GetHandle());
        }
#endif
    }

private:
    void OnNewTxnTextEnterHit(wxCommandEvent& event) override
    {
        event.Skip(false);
        if (m_requestPending)
        {
            return;
        }
        if (m_dataViewListCtrl6->GetItemCount() > 0 &&
            wxMessageBox(
                "Minta nomor transaksi baru dan hapus seluruh item lokal saat ini?",
                "Konfirmasi nomor transaksi baru",
                wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,
                this) != wxYES)
        {
            return;
        }
        ReserveNewTransaction(AfterNewTransaction::Manual);
    }

#ifdef __WXMSW__
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override
    {
        if (message == WM_CLIPBOARDUPDATE)
        {
            UpdateClipboardIndicator();
        }

        return Frame::MSWWindowProc(message, wParam, lParam);
    }
#endif

    void OnCharHook(wxKeyEvent& event)
    {
        if (event.GetKeyCode() == WXK_INSERT)
        {
            m_insertMode = !m_insertMode;
            m_statusBar1->SetStatusText(m_insertMode ? "INS" : "OVR", 1);
        }
        else if (event.GetKeyCode() == WXK_CAPITAL)
        {
            CallAfter(&ClientFrame::UpdateCapsIndicator);
        }

        event.Skip();
    }

    void OnSkuEnter(wxCommandEvent& event)
    {
        OnAddItem(event);
    }

    void OnOrderTypeChanged(wxCommandEvent&)
    {
        UpdateShippingControls();
    }

    void UpdateCapsIndicator()
    {
#ifdef __WXMSW__
        const bool capsLockOn = (GetKeyState(VK_CAPITAL) & 1) != 0;
        m_statusBar1->SetStatusText(capsLockOn ? "CAPS" : "caps", 2);
#endif
    }

    void UpdateClipboardIndicator()
    {
        if (!wxTheClipboard->Open())
        {
            m_statusBar1->SetStatusText("CLB?", 3);
            return;
        }

        const bool hasText = wxTheClipboard->IsSupported(wxDF_UNICODETEXT) ||
                             wxTheClipboard->IsSupported(wxDF_TEXT);
        wxTheClipboard->Close();
        m_statusBar1->SetStatusText(hasText ? "CLB" : "---", 3);
    }

    void AddItemListColumns()
    {
        m_dataViewListCtrl6->AppendTextColumn("No. Transaksi");
        m_dataViewListCtrl6->AppendTextColumn("No.");
        m_dataViewListCtrl6->AppendTextColumn("SKU");
        m_dataViewListCtrl6->AppendTextColumn("Jumlah");
        m_dataViewListCtrl6->AppendTextColumn("Harga");
        m_dataViewListCtrl6->AppendTextColumn("Sub-Total");
    }

    enum class RequestOperation
    {
        None,
        ServerStatus,
        ServerTime,
        SkuLookup,
        Checkout,
        NewTransaction
    };

    enum class AfterNewTransaction
    {
        Startup,
        Cancel,
        Manual,
        Checkout
    };

    struct OrderLine
    {
        wxString sku;
        int quantity;
    };

    void UpdateActionButtons()
    {
        const bool enabled = !m_requestPending;
        const bool hasTransaction = !m_textNewTxn->GetValue().empty();
        m_buttonTambahItem->Enable(enabled && hasTransaction);
        m_buttonCheckOut->Enable(enabled && hasTransaction &&
                                 m_dataViewListCtrl6->GetItemCount() > 0);
        m_btnServerSts->Enable(enabled);
        m_btnSvrTime->Enable(enabled);
        m_btnItemBatalAll->Enable(enabled && m_dataViewListCtrl6->GetItemCount() > 0);
        m_btnTxnBatal->Enable(enabled && hasTransaction);
    }

    bool StartPostRequest(const wxString& url, const wxString& body,
                          int requestId, RequestOperation operation)
    {
        if (m_requestPending)
        {
            return false;
        }
        if (!m_webSession.IsOpened())
        {
            m_statusBar1->SetStatusText("REST API tidak tersedia.", 0);
            wxMessageBox("Backend HTTP wxWidgets tidak tersedia.",
                         "REST API", wxOK | wxICON_ERROR, this);
            return false;
        }

        m_webRequest = m_webSession.CreateRequest(this, url, requestId);
        m_webRequest.SetMethod("POST");
        m_webRequest.SetHeader("Accept", "application/json");
        if (!body.empty())
        {
            m_webRequest.SetData(body, "application/json");
        }
        m_pendingOperation = operation;
        m_requestPending = true;
        UpdateActionButtons();
        m_webRequest.Start();
        return true;
    }

    void ReserveNewTransaction(AfterNewTransaction after)
    {
        m_afterNewTransaction = after;
        const bool reserve = after == AfterNewTransaction::Cancel ||
                             after == AfterNewTransaction::Manual;
        const wxString currentOrderId = m_textNewTxn->GetValue().Trim();
        const std::string body =
            "{\"reserve\":" + std::string(reserve ? "true" : "false") +
            ",\"previous_order_id\":\"" + EscapeJsonString(currentOrderId) + "\"}";
        m_statusBar1->SetStatusText(
            reserve ? "Mencadangkan nomor transaksi berikutnya..." :
                      "Memeriksa nomor transaksi berikutnya...",
            0);
        StartPostRequest(wxString::FromUTF8(kNewOrderUrl), wxString::FromUTF8(body),
                         kNewOrderRequestId,
                         RequestOperation::NewTransaction);
    }

    void OnServerStatus(wxCommandEvent&)
    {
        m_statusBar1->SetStatusText("Memeriksa status server...", 0);
        StartPostRequest(wxString::FromUTF8(kServerStatusUrl), "{}", kServerStatusRequestId,
                         RequestOperation::ServerStatus);
    }

    void OnServerTime(wxCommandEvent&)
    {
        m_statusBar1->SetStatusText("Meminta waktu server...", 0);
        StartPostRequest(wxString::FromUTF8(kServerTimeUrl), "{}", kServerTimeRequestId,
                         RequestOperation::ServerTime);
    }

    void OnExit(wxCommandEvent&)
    {
        if (wxMessageBox("Apakah Anda yakin ingin keluar dari aplikasi?",
                         "Konfirmasi keluar",
                         wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,
                         this) == wxYES)
        {
            Close(true);
        }
    }

    void ClearLocalItems()
    {
        while (m_dataViewListCtrl6->GetItemCount() > 0)
        {
            m_dataViewListCtrl6->DeleteItem(m_dataViewListCtrl6->GetItemCount() - 1);
        }
        m_orderLines.clear();
        UpdateActionButtons();
    }

    void ResetTransactionFields()
    {
        ClearLocalItems();
        m_textSKU->Clear();
        m_spinCtrl1->SetValue(1);
        m_textTipeSumber->Clear();
        m_textPenerima->Clear();
        m_textAlamat->Clear();
        m_textNoTelp->Clear();
        m_textNamaKurir->Clear();
        m_textOngkir->ChangeValue("0,00");
        m_radioBox1->SetSelection(0);
        UpdateShippingControls();
    }

    void OnClearItems(wxCommandEvent&)
    {
        if (wxMessageBox("Ya, hapus seluruh item dari daftar ini?",
                         "Konfirmasi hapus item",
                         wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,
                         this) == wxYES)
        {
            ClearLocalItems();
            m_statusBar1->SetStatusText("Seluruh item pada daftar lokal telah dihapus.", 0);
        }
    }

    void OnCancelTransaction(wxCommandEvent&)
    {
        if (wxMessageBox("Batalkan transaksi ini, hapus seluruh item pada daftar, dan minta nomor transaksi baru?",
                         "Konfirmasi pembatalan transaksi",
                         wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,
                         this) == wxYES)
        {
            ReserveNewTransaction(AfterNewTransaction::Cancel);
        }
    }

    void OnCheckout(wxCommandEvent&)
    {
        if (m_requestPending)
        {
            return;
        }

        if (m_dataViewListCtrl6->GetItemCount() == 0)
        {
            wxMessageBox("Tambahkan setidaknya satu item sebelum Check Out.",
                         "Pesanan kosong", wxOK | wxICON_WARNING, this);
            return;
        }

        const wxString transactionId = m_textNewTxn->GetValue().Trim();
        const bool isOnlineOrder = m_radioBox1->GetSelection() == 1;
        wxString source("OFFLINE_KASIR");
        wxString courier;
        if (isOnlineOrder)
        {
            wxString merchant = m_textTipeSumber->GetValue().Trim();
            merchant.MakeUpper();
            if (merchant.StartsWith("ONLINE_"))
            {
                merchant = merchant.Mid(7);
            }
            source = "ONLINE_" + merchant;

            courier = m_textNamaKurir->GetValue().Trim();
            courier.MakeUpper();
            if (courier.StartsWith("KURIR_"))
            {
                courier = courier.Mid(6);
            }
        }
        const wxString recipient = isOnlineOrder ? m_textPenerima->GetValue().Trim() : wxString();
        const wxString address = isOnlineOrder ? m_textAlamat->GetValue().Trim() : wxString();
        const wxString phone = isOnlineOrder ? m_textNoTelp->GetValue().Trim() : wxString();

        long long shippingCost = 0;

        if (transactionId.empty())
        {
            wxMessageBox("Nomor transaksi dari server belum tersedia.",
                         "Check Out", wxOK | wxICON_WARNING, this);
            return;
        }
        if (isOnlineOrder &&
            (source == "ONLINE_" || recipient.length() < 5 || phone.length() < 5 || address.length() < 5 ||
             courier.empty()))
        {
            wxMessageBox("Untuk pesanan online, nama E-Commerce, nama penerima, nomor telepon, alamat (minimal 5 karakter), dan kurir wajib diisi.",
                         "Data pengiriman belum lengkap",
                         wxOK | wxICON_WARNING, this);
            return;
        }
        if (isOnlineOrder && !TryParseRupiah(m_textOngkir->GetValue(), shippingCost))
        {
            wxMessageBox("Ongkos kirim harus berupa nominal rupiah non-negatif. Contoh: 20000 atau 20.000.",
                         "Ongkos kirim tidak valid",
                         wxOK | wxICON_WARNING, this);
            return;
        }

        for (std::size_t index = 0; index < m_orderLines.size(); ++index)
        {
            const OrderLine& line = m_orderLines[index];
            if (line.sku.empty() || line.quantity <= 0)
            {
                wxMessageBox(
                    wxString::Format("Item nomor %lu belum memiliki SKU yang benar dan jumlah lebih dari 0.",
                                     static_cast<unsigned long>(index + 1)),
                    "Item belum valid", wxOK | wxICON_WARNING, this);
                return;
            }
        }

        m_pendingTransactionId = transactionId;
        m_checkoutPickupType = isOnlineOrder
            ? "KURIR_" + courier
            : "CASHIER GERAI";
        m_checkoutSource = source;
        m_checkoutRecipient = recipient;
        m_checkoutPhone = phone;
        m_checkoutAddress = address;
        m_checkoutCourier = courier;
        m_checkoutShippingCost = shippingCost;
        std::string body =
            "{\"order_id\":\"" + EscapeJsonString(m_pendingTransactionId) +
            "\",\"tipe_sumber\":\"" + EscapeJsonString(m_checkoutSource) +
            "\",\"tipe_pengambilan\":\"" + EscapeJsonString(m_checkoutPickupType) +
            "\",\"nama_penerima\":\"" + EscapeJsonString(m_checkoutRecipient) +
            "\",\"no_telepon\":\"" + EscapeJsonString(m_checkoutPhone) +
            "\",\"alamat_kirim\":\"" + EscapeJsonString(m_checkoutAddress) +
            "\",\"kurir_ekspedisi\":\"" + EscapeJsonString(m_checkoutCourier) +
            "\",\"harga_ongkir\":" + std::to_string(m_checkoutShippingCost) +
            ",\"items\":[";
        for (std::size_t index = 0; index < m_orderLines.size(); ++index)
        {
            if (index != 0)
            {
                body += ',';
            }
            const OrderLine& line = m_orderLines[index];
            body += "{\"sku\":\"" + EscapeJsonString(line.sku) +
                    "\",\"jumlah\":" + std::to_string(line.quantity) + "}";
        }
        body += "]}";

        m_statusBar1->SetStatusText("Mengirim seluruh item dan Check Out secara atomik...", 0);
        StartPostRequest(wxString::FromUTF8(kCheckoutUrl), wxString::FromUTF8(body),
                         kCheckoutRequestId, RequestOperation::Checkout);
    }

    void OnAddItem(wxCommandEvent& event)
    {
        if (m_textNewTxn->GetValue().empty())
        {
            return;
        }

        const wxString sku = m_textSKU->GetValue().Trim();
        if (sku.empty())
        {
            wxMessageBox("Masukkan SKU terlebih dahulu.",
                         "SKU belum diisi", wxOK | wxICON_WARNING, this);
            return;
        }
        if (m_requestPending)
        {
            if (m_pendingOperation == RequestOperation::SkuLookup &&
                sku == m_pendingSku)
            {
                m_addWhenLookupCompletes = true;
            }
            return;
        }
        if (!m_skuLookupValid || sku != m_skuLookupSku)
        {
            m_addWhenLookupCompletes = true;
            StartSkuLookup(sku);
            return;
        }
        CommitCurrentItem();
        event.Skip();
    }

    bool CommitCurrentItem()
    {
        const wxString sku = m_textSKU->GetValue().Trim();
        const int quantity = m_spinCtrl1->GetValue();
        if (quantity <= 0)
        {
            wxMessageBox("Jumlah item harus lebih dari 0.",
                         "Jumlah tidak valid", wxOK | wxICON_WARNING, this);
            return false;
        }
        if (m_skuLookupStock <= 0)
        {
            wxMessageBox("Stok SKU ini habis. Item tidak dapat ditambahkan.",
                         "Stok habis", wxOK | wxICON_WARNING, this);
            return false;
        }

        const int existingQuantity = ExistingQuantityForSku(sku);
        if (quantity > m_skuLookupStock - existingQuantity)
        {
            wxMessageBox(
                wxString::Format("Stok SKU %s tidak mencukupi. Tersedia %d, sudah ada %d di daftar, diminta %d.",
                                 sku, m_skuLookupStock, existingQuantity, quantity),
                "Stok tidak mencukupi", wxOK | wxICON_WARNING, this);
            return false;
        }

        const double subtotal = m_skuLookupUnitPrice * quantity;
        m_orderLines.push_back({sku, quantity});

        wxVector<wxVariant> row;
        row.reserve(6);
        row.push_back(wxVariant(m_textNewTxn->GetValue()));
        row.push_back(wxVariant(wxString::Format("%lu",
            static_cast<unsigned long>(m_orderLines.size()))));
        row.push_back(wxVariant(sku));
        row.push_back(wxVariant(wxString::Format("%d", quantity)));
        row.push_back(wxVariant(wxString::Format("%.2f", m_skuLookupUnitPrice)));
        row.push_back(wxVariant(wxString::Format("%.2f", subtotal)));
        m_dataViewListCtrl6->AppendItem(row);

        m_textSKU->Clear();
        m_spinCtrl1->SetValue(1);
        m_statusBar1->SetStatusText(wxString::Format(
            "Item ditambahkan. Stok tersedia saat pengecekan: %d.", m_skuLookupStock), 0);
        m_skuLookupValid = false;
        UpdateActionButtons();
        return true;
    }

    int ExistingQuantityForSku(const wxString& sku) const
    {
        int total = 0;
        for (const OrderLine& line : m_orderLines)
        {
            if (line.sku == sku && line.quantity <= std::numeric_limits<int>::max() - total)
            {
                total += line.quantity;
            }
        }
        return total;
    }

    void StartSkuLookup(const wxString& sku)
    {
        if (sku.empty() || m_requestPending)
        {
            return;
        }
        const int quantity = m_spinCtrl1->GetValue();
        const std::string body =
            "{\"sku\":\"" + EscapeJsonString(sku) +
            "\",\"jumlah\":" + std::to_string(quantity) + "}";
        m_pendingSku = sku;
        m_statusBar1->SetStatusText("Memeriksa harga dan stok SKU...", 0);
        if (!StartPostRequest(wxString::FromUTF8(kSkuLookupUrl),
                              wxString::FromUTF8(body), kSkuLookupRequestId,
                              RequestOperation::SkuLookup))
        {
            m_pendingSku.clear();
        }
    }

    void OnSkuChanged(wxCommandEvent& event)
    {
        m_skuLookupValid = false;
        m_textSKU->UnsetToolTip();
        event.Skip();
    }

    void OnSkuFocusLost(wxFocusEvent& event)
    {
        event.Skip();
        const wxString sku = m_textSKU->GetValue().Trim();
        if (!sku.empty() && !m_skuLookupValid && !m_requestPending)
        {
            StartSkuLookup(sku);
        }
    }

    void OnQuantityChanged(wxSpinEvent& event)
    {
        UpdateSkuPreview();
        event.Skip();
    }

    void OnQuantityTextChanged(wxCommandEvent& event)
    {
        UpdateSkuPreview();
        event.Skip();
    }

    void UpdateSkuPreview()
    {
        if (!m_skuLookupValid)
        {
            return;
        }
        const int quantity = m_spinCtrl1->GetValue();
        const double subtotal = m_skuLookupUnitPrice * quantity;
        m_textSKU->SetToolTip(wxString::Format(
            "Harga normal: %.2f | Diskon: %.2f%% | Harga satuan: %.2f | Jumlah: %d | Subtotal: %.2f | Stok tersedia: %d",
            m_skuLookupNormalPrice, m_skuLookupDiscountPercent,
            m_skuLookupUnitPrice, quantity, subtotal, m_skuLookupStock));
        m_statusBar1->SetStatusText(wxString::Format(
            "Harga normal %.2f | Diskon %.2f%% | Harga %.2f | Subtotal %.2f | Stok %d",
            m_skuLookupNormalPrice, m_skuLookupDiscountPercent,
            m_skuLookupUnitPrice, subtotal, m_skuLookupStock), 0);
    }

    void OnWebRequestResponse(wxWebRequestEvent& event)
    {
        if (event.GetState() != wxWebRequest::State_Completed &&
            event.GetState() != wxWebRequest::State_Failed &&
            event.GetState() != wxWebRequest::State_Cancelled)
        {
            return;
        }

        if (event.GetState() != wxWebRequest::State_Completed)
        {
            const wxString error = event.GetErrorDescription().empty()
                ? "Tidak dapat terhubung ke REST API di 127.0.0.1:8868."
                : event.GetErrorDescription();
            const RequestOperation operation = m_pendingOperation;
            m_requestPending = false;
            m_pendingOperation = RequestOperation::None;
            if (operation == RequestOperation::SkuLookup)
            {
                m_addWhenLookupCompletes = false;
                m_pendingSku.clear();
            }
            UpdateActionButtons();
            m_statusBar1->SetStatusText("REST API gagal.", 0);
            const wxString title =
                operation == RequestOperation::Checkout
                    ? "Check Out gagal atau belum terkonfirmasi"
                    : "Request REST API gagal";
            const wxString detail = operation == RequestOperation::Checkout
                ? "\n\nServer memproses semua item dalam satu transaksi database. Jika koneksi terputus, periksa status transaksi sebelum mengirim ulang."
                : "";
            wxMessageBox(error + detail, title, wxOK | wxICON_ERROR, this);
            return;
        }

        const wxWebResponse response = event.GetResponse();
        const wxString responseBody = response.AsString();
        const RequestOperation operation = m_pendingOperation;
        m_requestPending = false;
        m_pendingOperation = RequestOperation::None;
        if (response.GetStatus() < 200 || response.GetStatus() >= 300)
        {
            if (operation == RequestOperation::SkuLookup)
            {
                m_addWhenLookupCompletes = false;
                m_pendingSku.clear();
            }
            UpdateActionButtons();
            m_statusBar1->SetStatusText("REST API mengembalikan kesalahan.", 0);
            wxMessageBox(responseBody.empty()
                             ? "Server mengembalikan respons kosong."
                             : responseBody,
                         "REST API",
                         wxOK | wxICON_ERROR,
                         this);
            return;
        }

        if (operation == RequestOperation::ServerStatus ||
            operation == RequestOperation::ServerTime)
        {
            UpdateActionButtons();
            m_statusBar1->SetStatusText("Server merespons.", 0);
            wxMessageBox(responseBody, operation == RequestOperation::ServerStatus
                                          ? "Status Server"
                                          : "Waktu Server",
                         wxOK | wxICON_INFORMATION, this);
            return;
        }

        if (operation == RequestOperation::SkuLookup)
        {
            const wxString requestedSku = m_pendingSku;
            m_pendingSku.clear();
            if (requestedSku != m_textSKU->GetValue().Trim())
            {
                m_addWhenLookupCompletes = false;
                UpdateActionButtons();
                m_statusBar1->SetStatusText(
                    "SKU berubah saat pencarian; ulangi pengecekan SKU.", 0);
                return;
            }
            if (!JsonSuccessIsTrue(responseBody))
            {
                m_addWhenLookupCompletes = false;
                m_skuLookupValid = false;
                UpdateActionButtons();
                m_statusBar1->SetStatusText("SKU tidak ditemukan atau tidak valid.", 0);
                wxMessageBox(responseBody.empty()
                                 ? "Server mengembalikan respons kosong atau tidak valid."
                                 : responseBody,
                             "Pengecekan SKU gagal",
                             wxOK | wxICON_WARNING, this);
                return;
            }
            double unitPrice = 0.0;
            double normalPrice = 0.0;
            double discountPercent = 0.0;
            double stockValue = 0.0;
            if (!TryGetJsonNumber(responseBody, "harga_normal", normalPrice) ||
                !TryGetJsonNumber(responseBody, "persentasi_diskon", discountPercent) ||
                !TryGetJsonNumber(responseBody, "harga_satuan", unitPrice) ||
                !TryGetJsonNumber(responseBody, "stok", stockValue) ||
                normalPrice <= 0.0 || discountPercent < 0.0 ||
                discountPercent > 100.0 || unitPrice < 0.0 || stockValue < 0.0 ||
                stockValue > std::numeric_limits<int>::max() ||
                std::floor(stockValue) != stockValue)
            {
                m_addWhenLookupCompletes = false;
                m_skuLookupValid = false;
                UpdateActionButtons();
                wxMessageBox("Respons harga atau stok dari server tidak valid.",
                             "Respons REST API tidak valid",
                             wxOK | wxICON_ERROR, this);
                return;
            }
            m_skuLookupSku = requestedSku;
            m_skuLookupNormalPrice = normalPrice;
            m_skuLookupDiscountPercent = discountPercent;
            m_skuLookupUnitPrice = unitPrice;
            m_skuLookupStock = static_cast<int>(stockValue);
            m_skuLookupValid = true;
            const int quantity = m_spinCtrl1->GetValue();
            const double subtotal = unitPrice * quantity;
            m_textSKU->SetToolTip(wxString::Format(
                "Harga normal: %.2f | Diskon: %.2f%% | Harga satuan: %.2f | Jumlah: %d | Subtotal: %.2f | Stok tersedia: %d",
                normalPrice, discountPercent, unitPrice, quantity, subtotal,
                m_skuLookupStock));
            m_statusBar1->SetStatusText(wxString::Format(
                "Harga normal %.2f | Diskon %.2f%% | Harga %.2f | Subtotal %.2f | Stok %d",
                normalPrice, discountPercent, unitPrice, subtotal, m_skuLookupStock), 0);
            const bool addAfterLookup = m_addWhenLookupCompletes;
            m_addWhenLookupCompletes = false;
            UpdateActionButtons();
            if (m_skuLookupStock == 0)
            {
                wxMessageBox("SKU ditemukan, tetapi stoknya habis.",
                             "Stok habis", wxOK | wxICON_WARNING, this);
            }
            if (addAfterLookup && m_skuLookupStock > 0)
            {
                CommitCurrentItem();
            }
            return;
        }

        if (!JsonSuccessIsTrue(responseBody))
        {
            UpdateActionButtons();
            m_statusBar1->SetStatusText("REST API menolak permintaan.", 0);
            wxMessageBox(responseBody.empty()
                             ? "Server mengembalikan respons kosong atau tidak valid."
                             : responseBody,
                         "REST API", wxOK | wxICON_ERROR, this);
            return;
        }

        if (operation == RequestOperation::NewTransaction)
        {
            wxString orderId;
            if (!TryGetJsonString(responseBody, "order_id", orderId))
            {
                UpdateActionButtons();
                m_statusBar1->SetStatusText("Nomor transaksi tidak valid.", 0);
                wxMessageBox("Server berhasil merespons tetapi tidak memberikan nomor transaksi.",
                             "Respons REST API tidak valid",
                             wxOK | wxICON_ERROR, this);
                return;
            }

            if (m_afterNewTransaction != AfterNewTransaction::Startup)
            {
                ResetTransactionFields();
            }
            m_textNewTxn->ChangeValue(orderId);
            m_statusBar1->SetStatusText(
                m_afterNewTransaction == AfterNewTransaction::Cancel
                    ? "Nomor transaksi baru dicadangkan server."
                    : m_afterNewTransaction == AfterNewTransaction::Manual
                        ? "Nomor transaksi baru dicadangkan server."
                        : "Nomor transaksi tersedia; belum dicadangkan.",
                0);
            UpdateActionButtons();
            return;
        }

        if (operation == RequestOperation::Checkout)
        {
            m_textNewTxn->Clear();
            ResetTransactionFields();
            m_statusBar1->SetStatusText("Check Out berhasil; menyiapkan transaksi berikutnya...", 0);
            wxMessageBox(responseBody, "Check Out berhasil",
                         wxOK | wxICON_INFORMATION, this);
            ReserveNewTransaction(AfterNewTransaction::Checkout);
            return;
        }

        UpdateActionButtons();
    }

    void UpdateShippingControls()
    {
        const bool isOnlineOrder = m_radioBox1->GetSelection() == 1;
        m_textTipeSumber->Enable(isOnlineOrder);
        m_textPenerima->Enable(isOnlineOrder);
        m_textPenerima->SetHint(
            isOnlineOrder ? "Ketik Nama Penerima" : wxString());
        m_textAlamat->Enable(isOnlineOrder);
        m_textNoTelp->Enable(isOnlineOrder);
        m_textNamaKurir->Enable(isOnlineOrder);
        m_textOngkir->Enable(isOnlineOrder);
        GetSizer()->Show(m_staticAlamatHint, isOnlineOrder);
        GetSizer()->Layout();
    }

    wxWebSession m_webSession{wxWebSession::New(wxWebSessionBackendWinHTTP)};
    wxWebRequest m_webRequest;
    wxString m_pendingTransactionId;
    wxString m_checkoutSource;
    wxString m_checkoutPickupType;
    wxString m_checkoutRecipient;
    wxString m_checkoutPhone;
    wxString m_checkoutAddress;
    wxString m_checkoutCourier;
    long long m_checkoutShippingCost{0};
    std::vector<OrderLine> m_orderLines;
    wxString m_pendingSku;
    wxString m_skuLookupSku;
    double m_skuLookupNormalPrice{0.0};
    double m_skuLookupDiscountPercent{0.0};
    double m_skuLookupUnitPrice{0.0};
    int m_skuLookupStock{0};
    RequestOperation m_pendingOperation{RequestOperation::None};
    AfterNewTransaction m_afterNewTransaction{AfterNewTransaction::Startup};
    bool m_requestPending{false};
    bool m_skuLookupValid{false};
    bool m_addWhenLookupCompletes{false};
    bool m_insertMode{true};
#ifdef __WXMSW__
    bool m_clipboardListenerRegistered{false};
#endif
};

class ClientApplication final : public wxApp
{
public:
    bool OnInit() override
    {
        wxInitAllImageHandlers();
        const wxString executablePath = wxStandardPaths::Get().GetExecutablePath();
        const wxString executableDirectory =
            wxFileName(executablePath).GetPath();
        if (!wxSetWorkingDirectory(executableDirectory))
        {
            wxLogError("Tidak dapat mengatur direktori kerja aplikasi: %s",
                       executableDirectory);
            return false;
        }

        auto* frame = new ClientFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(ClientApplication);
