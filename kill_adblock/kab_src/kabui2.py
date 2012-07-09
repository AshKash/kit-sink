#Boa:Dialog:Dialog1

import wx
import wx.stc

def create(parent):
    return Dialog1(parent)

[wxID_DIALOG1, wxID_DIALOG1CHECKBOX1, wxID_DIALOG1FILEPICKERCTRL1, 
 wxID_DIALOG1RADIOBUTTON1, wxID_DIALOG1RADIOBUTTON2, wxID_DIALOG1STATICLINE1, 
 wxID_DIALOG1STATICLINE2, wxID_DIALOG1STATICTEXT1, wxID_DIALOG1STATICTEXT2, 
 wxID_DIALOG1STATUSBAR1, wxID_DIALOG1STYLEDTEXTCTRL1, wxID_DIALOG1TEXTCTRL1, 
 wxID_DIALOG1TEXTCTRL2, 
] = [wx.NewId() for _init_ctrls in range(13)]

class Dialog1(wx.Dialog):
    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Dialog.__init__(self, id=wxID_DIALOG1, name='', parent=prnt,
              pos=wx.Point(327, 344), size=wx.Size(734, 771),
              style=wx.DEFAULT_DIALOG_STYLE, title='KAB encryption tool')
        self.SetClientSize(wx.Size(726, 731))

        self.staticLine1 = wx.StaticLine(id=wxID_DIALOG1STATICLINE1,
              name='staticLine1', parent=self, pos=wx.Point(104, 0),
              size=wx.Size(0, 2), style=0)

        self.styledTextCtrl1 = wx.stc.StyledTextCtrl(id=wxID_DIALOG1STYLEDTEXTCTRL1,
              name='styledTextCtrl1', parent=self, pos=wx.Point(0, 224),
              size=wx.Size(720, 480), style=0)
        self.styledTextCtrl1.SetAutoLayout(True)

        self.radioButton1 = wx.RadioButton(id=wxID_DIALOG1RADIOBUTTON1,
              label='Local file', name='radioButton1', parent=self,
              pos=wx.Point(16, 40), size=wx.Size(95, 16), style=0)
        self.radioButton1.SetValue(True)

        self.radioButton2 = wx.RadioButton(id=wxID_DIALOG1RADIOBUTTON2,
              label='Url', name='radioButton2', parent=self, pos=wx.Point(16,
              88), size=wx.Size(95, 16), style=0)
        self.radioButton2.SetValue(True)

        self.checkBox1 = wx.CheckBox(id=wxID_DIALOG1CHECKBOX1,
              label='Embed script', name='checkBox1', parent=self,
              pos=wx.Point(16, 168), size=wx.Size(144, 16), style=0)
        self.checkBox1.SetValue(True)

        self.staticLine2 = wx.StaticLine(id=wxID_DIALOG1STATICLINE2,
              name='staticLine2', parent=self, pos=wx.Point(24, 352),
              size=wx.Size(1, 2), style=0)

        self.statusBar1 = wx.StatusBar(id=wxID_DIALOG1STATUSBAR1,
              name='statusBar1', parent=self, style=0)

        self.filePickerCtrl1 = wx.FilePickerCtrl(id=wxID_DIALOG1FILEPICKERCTRL1,
              message='Select a folder', name='filePickerCtrl1', parent=self,
              path='', pos=wx.Point(152, 40), size=wx.Size(344, 20),
              style=wx.DIRP_DEFAULT_STYLE, wildcard='*.*')

        self.textCtrl1 = wx.TextCtrl(id=wxID_DIALOG1TEXTCTRL1, name='textCtrl1',
              parent=self, pos=wx.Point(144, 80), size=wx.Size(344, 24),
              style=0, value='http://finance.yahoo.com')

        self.staticText1 = wx.StaticText(id=wxID_DIALOG1STATICTEXT1,
              label='Encrypted HTML output', name='staticText1', parent=self,
              pos=wx.Point(8, 208), size=wx.Size(132, 16), style=0)

        self.staticText2 = wx.StaticText(id=wxID_DIALOG1STATICTEXT2,
              label='Password (hex)', name='staticText2', parent=self,
              pos=wx.Point(24, 128), size=wx.Size(89, 16), style=0)

        self.textCtrl2 = wx.TextCtrl(id=wxID_DIALOG1TEXTCTRL2, name='textCtrl2',
              parent=self, pos=wx.Point(144, 128), size=wx.Size(344, 24),
              style=0, value='DEADFACE123123123123123400000000')

    def __init__(self, parent):
        self._init_ctrls(parent)
