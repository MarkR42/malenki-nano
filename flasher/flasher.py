#!/usr/bin/env python3

import os
import wx

resource_bitmaps = {}

def load_resources():
    def load_bm(filename):
        with open(filename, 'rb') as f:
            i = wx.Image(f)
        return wx.Bitmap(i)
        
    resource_bitmaps['flame'] = load_bm('flame.png')

class MainWindow(wx.Frame):
    def __init__(self, parent, title):
        wx.Frame.__init__(self, parent, title=title, size=(800,600))

        self.CreateStatusBar() # A StatusBar in the bottom of the window

        # Setting up the menu.
        filemenu= wx.Menu()

        # wx.ID_ABOUT and wx.ID_EXIT are standard ids provided by wxWidgets.
        menuAbout = filemenu.Append(wx.ID_ABOUT, "&About"," Information about this program")
        menuExit = filemenu.Append(wx.ID_EXIT,"E&xit"," Terminate the program")

        # Creating the menubar.
        menuBar = wx.MenuBar()
        menuBar.Append(filemenu,"&File") # Adding the "filemenu" to the MenuBar
        self.SetMenuBar(menuBar)  # Adding the MenuBar to the Frame content.

        # Set events.
        self.Bind(wx.EVT_MENU, self.OnAbout, menuAbout)
        self.Bind(wx.EVT_MENU, self.OnExit, menuExit)

        self.CreateWidgets()

        self.Show(True)
        self.SetStatusText("Initial Status1")

    def OnAbout(self,e):
        # A message dialog box with an OK button. wx.OK is a standard ID in wxWidgets.
        dlg = wx.MessageDialog( self, "Malenki-Nano Firmware Flasher", "About Flasher", wx.OK)
        dlg.ShowModal() # Show it
        dlg.Destroy() # finally destroy it when finished.

    def OnExit(self,e):
        self.Close(True)  # Close the frame.
    
    def CreateWidgets(self):
        panel = wx.Panel(self)
        box = wx.BoxSizer(wx.VERTICAL)
        heading = wx.StaticText(panel, -1, "Malenki Nano Firmware Flasher")
        heading.SetSize(heading.GetBestSize())
        box.Add(heading, 0, flag=(wx.ALL | wx.EXPAND), border=10)
        
        button_flash = wx.Button(panel, wx.ID_ANY, "Flash")
        button_flash.SetBitmap(resource_bitmaps['flame'], dir=wx.TOP)
        button_flash.Bind(wx.EVT_BUTTON, self.DoFlash)
        button_unbind = wx.Button(panel, wx.ID_ANY, "Unbind\n(erase eeprom)")

        hbox = wx.BoxSizer(wx.HORIZONTAL)
        for b in (button_flash, button_unbind):
            hbox.Add(b, proportion=1, flag=(wx.ALL | wx.EXPAND), border=10)
        box.Add(hbox, flag=wx.EXPAND)
        # Gauge (progress bar)
        progress = wx.Gauge(panel)
        box.Add(progress, flag=(wx.ALL | wx.EXPAND), border=10)
        self.widget_progress = progress
        progress.SetRange(100)
        progress.SetValue(42)
        # Text area for messages
        messages = wx.TextCtrl(panel, style=wx.TE_MULTILINE | wx.TE_READONLY)
        box.Add(messages, flag=(wx.ALL | wx.EXPAND), border=10)
        self.widget_messages = messages
        panel.SetSizer(box)
        panel.Layout()

    def DoFlash(self,event):
        print("Flash pressed")
        self.SetStatusText("Flash Pressed")
        


app = wx.App(False)
load_resources()
frame = MainWindow(None, "Firmware Flasher")
app.MainLoop()
