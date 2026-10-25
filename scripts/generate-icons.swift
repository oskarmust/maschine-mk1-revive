import AppKit
let root=CommandLine.arguments.count>1 ? CommandLine.arguments[1]:FileManager.default.currentDirectoryPath
let fm=FileManager.default
let dir=root+"/mk1-menubar/AppIcon.iconset"
try fm.createDirectory(atPath:dir,withIntermediateDirectories:true)
func controller(_ rect:NSRect,mono:Bool=false) {
    let ctx=NSGraphicsContext.current!.cgContext
    ctx.saveGState();ctx.translateBy(x:rect.minX,y:rect.minY);ctx.scaleBy(x:rect.width/760,y:rect.height/580)
    let body=NSBezierPath(roundedRect:NSRect(x:0,y:0,width:760,height:580),xRadius:65,yRadius:65)
    (mono ? NSColor.black:NSColor(calibratedWhite:0.10,alpha:1)).setFill();body.fill()
    if !mono {NSColor(calibratedWhite:0.28,alpha:1).setStroke();body.lineWidth=9;body.stroke()}
    let mint=mono ? NSColor.clear:NSColor(calibratedRed:0.27,green:0.96,blue:0.76,alpha:1)
    for i in 0..<2 {
        let r=NSRect(x:105+CGFloat(i)*295,y:370,width:265,height:111)
        (mono ? NSColor.clear:mint.withAlphaComponent(0.2)).setFill();NSBezierPath(roundedRect:r,xRadius:12,yRadius:12).fill()
        if mono {NSGraphicsContext.current?.compositingOperation = .copy;NSColor.clear.setFill();NSBezierPath(rect:r).fill();NSGraphicsContext.current?.compositingOperation = .sourceOver}
        else {
            mint.setStroke();let p=NSBezierPath();p.lineWidth=7
            p.move(to:NSPoint(x:r.minX+20,y:r.midY))
            for k in 0..<9 {p.line(to:NSPoint(x:r.minX+25+CGFloat(k)*26,y:r.midY+(k==3 ? 28:(k==4 ? -30:0))))};p.stroke()
        }
    }
    for i in 0..<8 {let x=128+CGFloat(i)*73
        (mono ? NSColor.clear:NSColor(calibratedWhite:0.65,alpha:1)).setFill()
        NSBezierPath(ovalIn:NSRect(x:x,y:311,width:22,height:22)).fill()
    }
    for y in 0..<4 {for x in 0..<4 {
        let r=NSRect(x:410+CGFloat(x)*73,y:34+CGFloat(y)*62,width:57,height:48)
        let hot=(x==1&&y>=1)||(y==2&&x<3)
        (mono ? NSColor.clear:(hot ? mint:NSColor(calibratedRed:1,green:0.46,blue:0.24,alpha:1))).setFill()
        if mono {NSGraphicsContext.current?.compositingOperation = .copy}
        NSBezierPath(roundedRect:r,xRadius:9,yRadius:9).fill()
        NSGraphicsContext.current?.compositingOperation = .sourceOver
    }}
    if !mono {
        let attrs:[NSAttributedString.Key:Any]=[.font:NSFont.systemFont(ofSize:55,weight:.black),.foregroundColor:NSColor.white]
        ("MK1" as NSString).draw(at:NSPoint(x:58,y:166),withAttributes:attrs)
        let p=NSBezierPath();p.move(to:NSPoint(x:66,y:83));p.line(to:NSPoint(x:94,y:83));p.line(to:NSPoint(x:119,y:120));p.line(to:NSPoint(x:143,y:50));p.line(to:NSPoint(x:172,y:95));p.line(to:NSPoint(x:244,y:95));p.lineWidth=12;mint.setStroke();p.stroke()
    }
    ctx.restoreGState()
}
func png(_ size:Int,icon:Bool)->Data {
    let rep=NSBitmapImageRep(bitmapDataPlanes:nil,pixelsWide:size,pixelsHigh:size,bitsPerSample:8,samplesPerPixel:4,hasAlpha:true,isPlanar:false,colorSpaceName:.deviceRGB,bytesPerRow:0,bitsPerPixel:0)!
    NSGraphicsContext.saveGraphicsState();NSGraphicsContext.current=NSGraphicsContext(bitmapImageRep:rep)!
    if icon {
        let s=CGFloat(size);let bg=NSBezierPath(roundedRect:NSRect(x:s*0.035,y:s*0.035,width:s*0.93,height:s*0.93),xRadius:s*0.2,yRadius:s*0.2)
        NSGradient(starting:NSColor(calibratedRed:0.13,green:0.17,blue:0.23,alpha:1),ending:NSColor(calibratedRed:0.025,green:0.04,blue:0.06,alpha:1))!.draw(in:bg,angle:110)
        let shadow=NSShadow();shadow.shadowColor=NSColor.black.withAlphaComponent(0.6);shadow.shadowBlurRadius=s*0.04;shadow.shadowOffset=NSSize(width:0,height:-s*0.025);shadow.set()
        controller(NSRect(x:s*0.12,y:s*0.22,width:s*0.76,height:s*0.58))
    } else {controller(NSRect(x:0,y:CGFloat(size)*0.1,width:CGFloat(size),height:CGFloat(size)*0.8),mono:true)}
    NSGraphicsContext.restoreGraphicsState()
    return rep.representation(using:.png,properties:[:])!
}
for size in [16,32,128,256,512] {
    try png(size,icon:true).write(to:URL(fileURLWithPath:dir+"/icon_\(size)x\(size).png"))
    try png(size*2,icon:true).write(to:URL(fileURLWithPath:dir+"/icon_\(size)x\(size)@2x.png"))
}
try png(1024,icon:true).write(to:URL(fileURLWithPath:root+"/mk1-menubar/AppIcon.png"))
try png(22,icon:false).write(to:URL(fileURLWithPath:root+"/mk1-menubar/menubar.png"))
try png(44,icon:false).write(to:URL(fileURLWithPath:root+"/mk1-menubar/menubar@2x.png"))
