///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "Frame_Base.h"

///////////////////////////////////////////////////////////////////////////

Frame::Frame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Segoe UI") ) );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );

	m_staticNewTransaksi = new wxStaticText( this, wxID_ANY, _("Tambah Transaksi"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticNewTransaksi->Wrap( -1 );
	m_staticNewTransaksi->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Segoe UI") ) );

	bSizer1->Add( m_staticNewTransaksi, 0, wxALL, 5 );

	m_textNewTxn = new wxTextCtrl( this, wxID_ANY, _("TXN20261020-0000"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	bSizer1->Add( m_textNewTxn, 1, wxALL, 5 );


	bSizer1->Add( 0, 0, 0, 0, 5 );

	m_staticSKU = new wxStaticText( this, wxID_ANY, _("Input SKU"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticSKU->Wrap( -1 );
	m_staticSKU->SetFont( wxFont( 9, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Segoe UI") ) );

	bSizer1->Add( m_staticSKU, 0, wxALL, 5 );

	m_textSKU = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_textSKU->SetFont( wxFont( 9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial Narrow") ) );

	bSizer1->Add( m_textSKU, 1, wxALL, 5 );

	m_staticJumlah = new wxStaticText( this, wxID_ANY, _("Jumlah"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticJumlah->Wrap( -1 );
	bSizer1->Add( m_staticJumlah, 0, wxALL, 5 );

	m_spinCtrl1 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10000, 1 );
	m_spinCtrl1->SetMinSize( wxSize( 60,-1 ) );

	bSizer1->Add( m_spinCtrl1, 0, wxALL, 5 );

	m_buttonTambahItem = new wxButton( this, wxID_ANY, _("Tambah Item"), wxDefaultPosition, wxDefaultSize, 0 );
	m_buttonTambahItem->SetFont( wxFont( 9, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Segoe UI") ) );
	m_buttonTambahItem->SetMinSize( wxSize( 85,-1 ) );

	bSizer1->Add( m_buttonTambahItem, 0, wxALL, 5 );


	bSizer4->Add( bSizer1, 0, wxEXPAND, 5 );

	wxFlexGridSizer* gSizer1;
	gSizer1 = new wxFlexGridSizer( 0, 0, 0, 0 );
	gSizer1->SetFlexibleDirection( wxHORIZONTAL );
	gSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxString m_radioBox1Choices[] = { _("Ke-Kasir"), _("Via On Line") };
	int m_radioBox1NChoices = sizeof( m_radioBox1Choices ) / sizeof( wxString );
	m_radioBox1 = new wxRadioBox( this, wxID_ANY, _("Cara Pesanan Masuk"), wxDefaultPosition, wxDefaultSize, m_radioBox1NChoices, m_radioBox1Choices, 1, wxRA_SPECIFY_ROWS );
	m_radioBox1->SetSelection( 0 );
	gSizer1->Add( m_radioBox1, 0, wxALL|wxEXPAND, 5 );

	m_textTipeSumber = new wxTextCtrl( this, wxID_ANY, _("Ketik Nama E-Commerce"), wxPoint( -1,-1 ), wxDefaultSize, 0 );
	m_textTipeSumber->Enable( false );

	gSizer1->Add( m_textTipeSumber, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );

	m_staticNamaKurir = new wxStaticText( this, wxID_ANY, _("Nama Kurir"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_staticNamaKurir->Wrap( -1 );
	bSizer6->Add( m_staticNamaKurir, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_textNamaKurir = new wxTextCtrl( this, wxID_ANY, _("Ketik nama kurir paket (Misalkan: SPX)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textNamaKurir->Enable( false );

	bSizer6->Add( m_textNamaKurir, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_staticOngkir = new wxStaticText( this, wxID_ANY, _("Ongkos Kirim"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_staticOngkir->Wrap( -1 );
	bSizer6->Add( m_staticOngkir, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_textOngkir = new wxTextCtrl( this, wxID_ANY, _("0,00"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT );
	#ifdef __WXGTK__
	if ( !m_textOngkir->HasFlag( wxTE_MULTILINE ) )
	{
	m_textOngkir->SetMaxLength( 9 );
	}
	#else
	m_textOngkir->SetMaxLength( 9 );
	#endif
	m_textOngkir->Enable( false );

	bSizer6->Add( m_textOngkir, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	gSizer1->Add( bSizer6, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED, 5 );


	bSizer4->Add( gSizer1, 0, wxEXPAND, 5 );

	m_staticAlamatHint = new wxStaticText( this, wxID_ANY, _("Ketik data alamat penerima termasuk Google Maps Koordinat"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_staticAlamatHint->Wrap( -1 );
	bSizer4->Add( m_staticAlamatHint, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxHORIZONTAL );

	m_staticPenerima = new wxStaticText( this, wxID_ANY, _("Nama Penerima"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_staticPenerima->Wrap( -1 );
	bSizer51->Add( m_staticPenerima, 0, wxALL|wxEXPAND, 5 );

	m_textPenerima = new wxTextCtrl( this, wxID_ANY, _("Ketik Nama Penerima"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textPenerima->Enable( false );

	bSizer51->Add( m_textPenerima, 1, wxALL, 5 );

	m_staticAlamat = new wxStaticText( this, wxID_ANY, _("Alamat Penerima:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticAlamat->Wrap( -1 );
	m_staticAlamat->SetMaxSize( wxSize( 100,-1 ) );

	bSizer51->Add( m_staticAlamat, 0, wxALL|wxEXPAND, 5 );

	m_textAlamat = new wxTextCtrl( this, wxID_ANY, _("Ketik data alamat penerima termasuk Google Maps Koordinat"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	m_textAlamat->Enable( false );
	m_textAlamat->SetMaxSize( wxSize( -1,40 ) );

	bSizer51->Add( m_textAlamat, 1, wxALL, 5 );


	bSizer4->Add( bSizer51, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer53;
	bSizer53 = new wxBoxSizer( wxHORIZONTAL );

	m_staticNoTelp = new wxStaticText( this, wxID_ANY, _("No. Telp. Penerima"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_staticNoTelp->Wrap( -1 );
	bSizer53->Add( m_staticNoTelp, 0, wxALL|wxEXPAND, 5 );

	m_textNoTelp = new wxTextCtrl( this, wxID_ANY, _("Ketik nomor telpon penerima"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textNoTelp->Enable( false );

	bSizer53->Add( m_textNoTelp, 0, wxALL|wxEXPAND, 5 );


	bSizer4->Add( bSizer53, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	m_dataViewListCtrl6 = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES|wxDV_MULTIPLE|wxDV_ROW_LINES|wxBORDER_DEFAULT|wxBORDER_THEME|wxHSCROLL|wxTRANSPARENT_WINDOW|wxVSCROLL );
	bSizer9->Add( m_dataViewListCtrl6, 1, wxALL|wxEXPAND, 5 );


	bSizer4->Add( bSizer9, 1, wxEXPAND, 5 );

	wxWrapSizer* wSizer1;
	wSizer1 = new wxWrapSizer( wxHORIZONTAL, wxWRAPSIZER_DEFAULT_FLAGS );

	m_buttonCheckOut = new wxButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );

	m_buttonCheckOut->SetBitmapDisabled( wxBitmap( wxT("disabled.png"), wxBITMAP_TYPE_ANY ) );
	m_buttonCheckOut->SetBitmapPressed( wxBitmap( wxT("pressed.png"), wxBITMAP_TYPE_ANY ) );
	m_buttonCheckOut->SetBitmapFocus( wxBitmap( wxT("current.png"), wxBITMAP_TYPE_ANY ) );
	m_buttonCheckOut->SetBitmapCurrent( wxBitmap( wxT("focus.png"), wxBITMAP_TYPE_ANY ) );
	m_buttonCheckOut->SetMinSize( wxSize( 128,36 ) );

	wSizer1->Add( m_buttonCheckOut, 0, wxALL, 5 );


	wSizer1->Add( 0, 0, 1, wxEXPAND, 5 );

	m_btnItemBatalAll = new wxButton( this, wxID_ANY, _("Item Hapus Smua"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizer1->Add( m_btnItemBatalAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_btnTxnBatal = new wxButton( this, wxID_ANY, _("Transaksi Dibatalkan"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizer1->Add( m_btnTxnBatal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_btnServerSts = new wxButton( this, wxID_ANY, _("Server Status"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizer1->Add( m_btnServerSts, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_btnSvrTime = new wxButton( this, wxID_ANY, _("Waktu Server"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizer1->Add( m_btnSvrTime, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	wSizer1->Add( 0, 0, 1, wxEXPAND, 5 );

	m_btnKeluarKonf = new wxButton( this, wxID_ANY, _("Keluar Program"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizer1->Add( m_btnKeluarKonf, 0, wxALL|wxEXPAND, 5 );


	bSizer4->Add( wSizer1, 0, wxEXPAND, 2 );


	this->SetSizer( bSizer4 );
	this->Layout();
	m_statusBar1 = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( Frame::FrameOnLeftDown ) );
	this->Connect( wxEVT_SHOW, wxShowEventHandler( Frame::FrameOnShow ) );
	m_textNewTxn->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( Frame::OnNewTxnTextEnterHit ), NULL, this );
}

Frame::~Frame()
{
}
