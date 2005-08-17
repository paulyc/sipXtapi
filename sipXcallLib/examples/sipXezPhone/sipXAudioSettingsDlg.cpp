/////////////////////////////////////////////////////////////////////////////
// Name:        sipXAudioSettingsDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "sipXAudioSettingsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "sipXAudioSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"

// WDR: class implementations

static wxListBox* codecListControl = NULL;

static wxString bwChoices[] = {
    "Low Bandwidth", "Normal Bandwidth", "High Bandwidth"
};


BEGIN_EVENT_TABLE(sipXAudioSettingsDlg,wxDialog)
    EVT_BUTTON( wxID_OK, sipXAudioSettingsDlg::OnOk )
    EVT_BUTTON( wxID_CANCEL, sipXAudioSettingsDlg::OnCancel )
    EVT_CHOICE( ID_BANDWIDTH_CHOICE, sipXAudioSettingsDlg::OnCodec )
END_EVENT_TABLE()


wxSizer *sipXAudioSettingsDlgFunc( wxWindow *parent, bool call_fit, bool set_sizer )
{
    codecListControl = NULL;
    wxGridSizer *grid0 = new wxGridSizer( 1, 0, 0 );

    wxStaticBox *audio = new wxStaticBox(parent, -1, wxT("Audio"), wxDefaultPosition, wxSize(314, 75), wxALIGN_LEFT);

    wxStaticText *item00 = new wxStaticText(parent, ID_TEXT, wxT("Enable echo cancellation?"), wxPoint(10,20), wxSize(130,20), wxALIGN_LEFT );
    wxCheckBox *item01 = new wxCheckBox(parent, ID_ENABLE_AEC, wxT(""), wxPoint(150,20), wxDefaultSize, 0 );
    item01->SetValue(sipXmgr::getInstance().isAECEnabled());

    wxStaticText *item2 = new wxStaticText(parent, ID_TEXT, wxT("Enable ouf-of-band DTMF?"), wxPoint(10,45), wxSize(130, 20), wxALIGN_LEFT );
    wxCheckBox *item3 = new wxCheckBox(parent, ID_ENABLE_OUT_DTMF, wxT(""), wxPoint(150,45), wxDefaultSize, 0 );
    item3->SetValue(sipXmgr::getInstance().isOutOfBandDTMFEnabled());

    wxStaticText *item10 = new wxStaticText(parent, ID_TEXT, wxT("Enable SRTP?"), wxPoint(190,20), wxSize(120, 20), wxALIGN_LEFT );
    wxCheckBox *item11 = new wxCheckBox(parent, ID_ENABLE_SRTP, wxT(""), wxPoint(290,20), wxDefaultSize, 0 );
    item11->SetValue(sipXmgr::getInstance().isSRTPEnabled());

    wxStaticBox *codecs = new wxStaticBox(parent, -1, wxT("Codecs"), wxPoint(-1,80), wxSize(314, 180), wxALIGN_LEFT);

    wxStaticText *item4 = new wxStaticText(parent, ID_TEXT, wxT("Codec preferences"), wxPoint(10,100), wxSize(180, 20), wxALIGN_LEFT );
    wxChoice *item5 = new wxChoice(parent, ID_BANDWIDTH_CHOICE, wxPoint(180,98), wxSize(-1,-1), 3, bwChoices);

    int bandWidth;
    if (sipXmgr::getInstance().getCodecPreferences(&bandWidth))
    {
        item5->SetSelection(bandWidth-1);
    }

    wxStaticText *item7 = new wxStaticText(parent, ID_TEXT, wxT("Supported codecs for this codec preference\n"), wxPoint(10,130), wxSize(120,-1), wxALIGN_LEFT );

    UtlString sData = "*sipXtapi error*";
    sipXmgr::getInstance().getCodecList(sData);

    wxListBox *item8 = new wxListBox(parent, ID_TEXT, wxPoint(140,130), wxSize(160, 120), 0, NULL, wxLB_ALWAYS_SB);
    codecListControl = item8;

    char *tokTmp;
    char *str = (char*)sData.data();
    
    tokTmp = strtok(str, "\n");

    while (tokTmp != NULL)
    {
        item8->Append(wxT(tokTmp));
        tokTmp = strtok(NULL, "\n");
    }

    wxButton *item9 = new wxButton(parent, wxID_OK, wxT("OK"), wxPoint(200, 270), wxDefaultSize);
    
    return grid0;
}

//----------------------------------------------------------------------------
// sipXAudioSettingsDlg
//----------------------------------------------------------------------------

sipXAudioSettingsDlg::sipXAudioSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function sipXAudioSettingsDlgFunc for sipXAudioSettingsDlg
    sipXAudioSettingsDlgFunc(this, TRUE, TRUE ); 

}

// WDR: handler implementations for sipXAudioSettingsDlg

void sipXAudioSettingsDlg::OnOk(wxCommandEvent &event)
{
    wxString x;
    wxCheckBox* pCheck;
    wxChoice* pChoice;
    int pos;
    bool b;

    pCheck = (wxCheckBox*)sipXAudioSettingsDlg::FindWindowById(ID_ENABLE_OUT_DTMF, this);
    b = pCheck->GetValue();
    sipXmgr::getInstance().enableOutOfBandDTMF(b);
    sipXezPhoneSettings::getInstance().setEnableOOBDTMF(b);

    pCheck = (wxCheckBox*)sipXAudioSettingsDlg::FindWindowById(ID_ENABLE_AEC, this);
    b = pCheck->GetValue();
    sipXmgr::getInstance().enableAEC(b);
    sipXezPhoneSettings::getInstance().setEnableAEC(b);

    pCheck = (wxCheckBox*)sipXAudioSettingsDlg::FindWindowById(ID_ENABLE_SRTP, this);
    b = pCheck->GetValue();
    sipXmgr::getInstance().enableSRTP(b);
    sipXezPhoneSettings::getInstance().setEnableSRTP(b);

    pChoice = (wxChoice*)sipXAudioSettingsDlg::FindWindowById(ID_BANDWIDTH_CHOICE, this);
    pos = pChoice->GetSelection();
    sipXmgr::getInstance().setCodecPreferences(pos+1);
    sipXezPhoneSettings::getInstance().setCodecPref(pos+1);

    sipXezPhoneSettings::getInstance().saveSettings();
    
    event.Skip();
}

void sipXAudioSettingsDlg::OnCancel(wxCommandEvent &event)
{
    event.Skip();
}

void sipXAudioSettingsDlg::OnCodec(wxCommandEvent &event)
{
    int i = event.GetSelection();

    if (codecListControl)
    {
        sipXmgr::getInstance().setCodecPreferences(i+1);
        codecListControl->Clear();

        UtlString sData = "*sipXtapi error*";
        sipXmgr::getInstance().getCodecList(sData);

        char *tokTmp;
        char *str = (char*)sData.data();
    
        tokTmp = strtok(str, "\n");

        while (tokTmp != NULL)
        {
            codecListControl->Append(wxT(tokTmp));
            tokTmp = strtok(NULL, "\n");
        }
    }
}




