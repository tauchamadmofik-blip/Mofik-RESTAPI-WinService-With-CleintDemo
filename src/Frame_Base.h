///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/dataview.h>
#include <wx/wrapsizer.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class Frame
///////////////////////////////////////////////////////////////////////////////
class Frame : public wxFrame
{
	private:

	protected:
		wxStaticText* m_staticNewTransaksi;
		wxTextCtrl* m_textNewTxn;
		wxStaticText* m_staticSKU;
		wxTextCtrl* m_textSKU;
		wxStaticText* m_staticJumlah;
		wxSpinCtrl* m_spinCtrl1;
		wxButton* m_buttonTambahItem;
		wxRadioBox* m_radioBox1;
		wxTextCtrl* m_textTipeSumber;
		wxStaticText* m_staticNamaKurir;
		wxTextCtrl* m_textNamaKurir;
		wxStaticText* m_staticOngkir;
		wxTextCtrl* m_textOngkir;
		wxStaticText* m_staticAlamatHint;
		wxStaticText* m_staticPenerima;
		wxTextCtrl* m_textPenerima;
		wxStaticText* m_staticAlamat;
		wxTextCtrl* m_textAlamat;
		wxStaticText* m_staticNoTelp;
		wxTextCtrl* m_textNoTelp;
		wxDataViewListCtrl* m_dataViewListCtrl6;
		wxButton* m_buttonCheckOut;
		wxButton* m_btnItemBatalAll;
		wxButton* m_btnTxnBatal;
		wxButton* m_btnServerSts;
		wxButton* m_btnSvrTime;
		wxButton* m_btnKeluarKonf;
		wxStatusBar* m_statusBar1;

		// Virtual event handlers, override them in your derived class
		virtual void FrameOnLeftDown( wxMouseEvent& event ) { event.Skip(); }
		virtual void FrameOnShow( wxShowEvent& event ) { event.Skip(); }
		virtual void OnNewTxnTextEnterHit( wxCommandEvent& event ) { event.Skip(); }


	public:

		Frame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Sample RESTFul KidsWear Client  Demo"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 761,573 ), long style = wxCAPTION|wxCLOSE_BOX|wxDEFAULT_FRAME_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxSTAY_ON_TOP|wxTAB_TRAVERSAL );

		~Frame();

};

