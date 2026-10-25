import AppKit
import Darwin

private let studioDir = NSHomeDirectory()+"/Library/Application Support/MK1 Revive"
private let knobNames = ["Volume", "Tempo", "Swing"] + (1...8).map { "Screen encoder \($0)" }
private let cyan = NSColor(calibratedRed:0.23, green:0.92, blue:0.78, alpha:1)
private let orange = NSColor(calibratedRed:1, green:0.52, blue:0.26, alpha:1)
private let ink = NSColor(calibratedWhite:0.055,alpha:1)

struct StudioPad: Codable {
    var `override` = false
    var enabled = true
    var on = 300
    var off = 150
    var pressure = 200
    var gain = 100
    var target: Int
}
struct StudioEncoder: Codable {
    var enabled = true
    var invert = false
    var gain = 100
    var target: Int
}
struct StudioProfile: Codable {
    var pads = (0..<16).map { StudioPad(target:$0) }
    var encoders = (0..<11).map { StudioEncoder(target:$0) }
}

@discardableResult
private func studioCommand(_ command:String)->Bool {
    let fd = Darwin.socket(AF_UNIX, SOCK_DGRAM, 0)
    guard fd >= 0 else { return false }
    defer { Darwin.close(fd) }
    var addr = sockaddr_un(); addr.sun_family = sa_family_t(AF_UNIX)
    let bytes = Array((studioDir+"/control.sock").utf8)+[0]
    guard bytes.count <= MemoryLayout.size(ofValue:addr.sun_path) else { return false }
    withUnsafeMutableBytes(of:&addr.sun_path) { ptr in ptr.copyBytes(from:bytes) }
    return command.withCString { text in
        withUnsafePointer(to:&addr) { p in
            p.withMemoryRebound(to:sockaddr.self,capacity:1) {
                Darwin.sendto(fd,text,strlen(text),0,$0,socklen_t(MemoryLayout<sockaddr_un>.size)) >= 0
            }
        }
    }
}

private struct HardwarePart {
    let rect:NSRect
    let kind:String
    let index:Int
    let title:String
}

final class MK1LayoutView:NSView {
    override var isFlipped:Bool { true }
    var state:[String:Any] = [:] { didSet { needsDisplay = true } }
    var selectedKind = "pad"
    var selectedIndex = 13
    var onSelect:((String,Int)->Void)?
    private var parts:[HardwarePart] = []

    private func label(_ s:String,_ rect:NSRect,_ size:CGFloat=10,_ color:NSColor = .secondaryLabelColor,_ weight:NSFont.Weight = .medium) {
        let style=NSMutableParagraphStyle();style.alignment = .center
        (s as NSString).draw(in:rect,withAttributes:[.font:NSFont.systemFont(ofSize:size,weight:weight),.foregroundColor:color,.paragraphStyle:style])
    }
    private func box(_ r:NSRect,_ color:NSColor,_ radius:CGFloat=7) {
        color.setFill();NSBezierPath(roundedRect:r,xRadius:radius,yRadius:radius).fill()
    }
    override func draw(_ dirtyRect:NSRect) {
        super.draw(dirtyRect)
        parts=[]
        let scale=min(bounds.width/760,bounds.height/590)
        NSGraphicsContext.saveGraphicsState()
        let transform=NSAffineTransform();transform.scale(by:scale);transform.concat()
        box(NSRect(x:0,y:0,width:760,height:590),NSColor(calibratedWhite:0.10,alpha:1),22)
        box(NSRect(x:2,y:2,width:756,height:586),NSColor(calibratedWhite:0.115,alpha:1),20)
        label("MASCHINE  MK1",NSRect(x:20,y:16,width:205,height:25),20,.white,.bold)
        label("REVIVE / LIVE INPUT",NSRect(x:520,y:20,width:216,height:18),10,cyan,.semibold)
        let pressure=(state["pads"] as? [Int]) ?? Array(repeating:0,count:16)
        let age=(state["encoderAge"] as? [Int]) ?? Array(repeating:999999,count:11)
        let delta=(state["encoderDelta"] as? [Int]) ?? Array(repeating:0,count:11)
        let buttons=(state["buttons"] as? NSNumber)?.uint64Value ?? 0
        func button(_ x:CGFloat,_ y:CGFloat,_ w:CGFloat,_ name:String,_ id:Int) {
            let r=NSRect(x:x,y:y,width:w,height:25)
            let active=(buttons & (1<<id)) != 0
            box(r,active ? orange:NSColor(calibratedWhite:0.19,alpha:1),4)
            label(name,NSRect(x:x,y:y+6,width:w,height:16),8,active ? ink:.lightGray,.semibold)
            parts.append(HardwarePart(rect:r,kind:"button",index:id,title:name))
        }
        // Two displays, eight screen buttons, eight corresponding encoders.
        for i in 0..<8 { button(151+CGFloat(i)*70,56,56,"\(i+1)",39-i) }
        for i in 0..<2 {
            let r=NSRect(x:145+CGFloat(i)*285,y:94,width:270,height:73)
            box(r,NSColor(calibratedRed:0.16,green:0.23,blue:0.23,alpha:1),6)
            let game=(state["game"] as? Int) ?? 0
            let names=["PONG","FREEDOOM","BREAKOUT"]
            let idle=(state["idle"] as? Bool) ?? false
            label(i==0 ? (idle ? names[max(0,min(2,game))]:"MASCHINE") : "MK1 REVIVE",NSRect(x:r.minX+8,y:r.minY+14,width:r.width-16,height:22),17,cyan,.semibold)
            label(i==0 ? "255 × 64 · LEFT LCD":"255 × 64 · RIGHT LCD",NSRect(x:r.minX,y:r.minY+48,width:r.width,height:15),9,.lightGray)
        }
        func knob(_ x:CGFloat,_ y:CGFloat,_ id:Int,_ title:String) {
            let r=NSRect(x:x-19,y:y-19,width:38,height:38)
            let active=age.count>id && age[id]<450
            box(r,active ? cyan:NSColor(calibratedWhite:0.055,alpha:1),19)
            let selected=selectedKind=="encoder" && selectedIndex==id
            (selected ? orange:NSColor(calibratedWhite:0.34,alpha:1)).setStroke()
            let path=NSBezierPath(ovalIn:r.insetBy(dx:1,dy:1));path.lineWidth=selected ? 3:1.3;path.stroke()
            let angle=CGFloat((delta.count>id ? delta[id]:0))*0.001
            let tick=NSBezierPath();tick.move(to:NSPoint(x:x,y:y));tick.line(to:NSPoint(x:x+sin(angle)*13,y:y-cos(angle)*13));tick.lineWidth=2
            (active ? ink:.white).setStroke();tick.stroke()
            label(title,NSRect(x:x-31,y:y+24,width:62,height:24),9,.lightGray)
            parts.append(HardwarePart(rect:r.insetBy(dx:-6,dy:-6),kind:"encoder",index:id,title:title))
        }
        for i in 0..<8 { knob(179+CGFloat(i)*70,201,i+3,"\(i+1)") }
        button(23,56,56,"CONTROL",24);button(85,56,56,"STEP",31)
        button(23,106,56,"BROWSE",25);button(85,106,56,"SAMPLING",30)
        button(23,148,56,"◀",26);button(85,148,56,"▶",29)
        button(23,190,56,"SNAP",27);button(85,190,56,"AUTO WRITE",28)
        // Right-hand 4x4 pad grid: printed numbers increase from bottom to top.
        for row in 0..<4 { for col in 0..<4 {
            let index=(3-row)*4+col
            let r=NSRect(x:413+CGFloat(col)*77,y:284+CGFloat(row)*67,width:65,height:57)
            let value=pressure.count>index ? pressure[index]:0
            let active=value>120
            box(r,active ? cyan:NSColor(calibratedWhite:0.22,alpha:1),7)
            if selectedKind=="pad" && selectedIndex==index {
                orange.setStroke();let p=NSBezierPath(roundedRect:r.insetBy(dx:-2,dy:-2),xRadius:9,yRadius:9);p.lineWidth=2;p.stroke()
            }
            label("\(index+1)",NSRect(x:r.minX,y:r.minY+14,width:r.width,height:24),19,active ? ink:.white,.bold)
            if value>0 { box(NSRect(x:r.minX+5,y:r.maxY-6,width:(r.width-10)*CGFloat(min(4095,value))/4095,height:2),orange,1) }
            parts.append(HardwarePart(rect:r,kind:"pad",index:index,title:"Pad \(index+1)"))
        } }
        // The three master encoders, groups and transport occupy the left half.
        knob(51,298,0,"VOLUME");knob(126,298,1,"TEMPO");knob(201,298,2,"SWING")
        button(250,285,64,"NOTE REPEAT",40)
        for row in 0..<2 { for col in 0..<4 {
            let i=row*4+col
            let ids=[23,22,21,20,17,16,18,19]
            button(23+CGFloat(col)*73,391+CGFloat(row)*35,61,String(UnicodeScalar(65+i)!),ids[i])
        } }
        let rowNames=["SCENE","PATTERN","PAD MODE","NAVIGATE","DUPLICATE","SELECT","SOLO","MUTE"]
        for i in 0..<8 { button(323,270+CGFloat(i)*33,70,rowNames[i],7-i) }
        button(23,493,61,"RESTART",15);button(96,493,61,"◀",14)
        button(169,493,61,"▶",13);button(242,493,61,"GRID",12)
        button(23,529,61,"PLAY",41);button(96,529,61,"REC",9)
        button(169,529,61,"ERASE",10);button(242,529,61,"SHIFT",11)
        label("Click a pad or knob to tune it · hardware input highlights live",NSRect(x:30,y:566,width:700,height:16),10,.gray)
        NSGraphicsContext.restoreGraphicsState()
    }
    override func mouseDown(with event:NSEvent) {
        let scale=min(bounds.width/760,bounds.height/590)
        let p=convert(event.locationInWindow,from:nil)
        let point=NSPoint(x:p.x/scale,y:p.y/scale)
        guard let part=parts.last(where:{$0.rect.contains(point)}) else { return }
        selectedKind=part.kind;selectedIndex=part.index;needsDisplay=true
        onSelect?(part.kind,part.index)
    }
}

final class ControllerStudio:NSObject,NSWindowDelegate {
    static let shared=ControllerStudio()
    private var window:NSWindow?
    private let canvas=MK1LayoutView()
    private var state:[String:Any]=[:]
    private var profile=StudioProfile()
    private var timer:Timer?
    private var selectedKind="pad", selectedIndex=13
    private var titleLabel=NSTextField(labelWithString:"")
    private var rawLabel=NSTextField(labelWithString:"")
    private var statusLabel=NSTextField(labelWithString:"Connecting to bridge…")
    private var notice=NSTextField(wrappingLabelWithString:"")
    private var enabled=NSButton(checkboxWithTitle:"Enabled",target:nil,action:nil)
    private var override=NSButton(checkboxWithTitle:"Override global thresholds",target:nil,action:nil)
    private var invert=NSButton(checkboxWithTitle:"Invert direction",target:nil,action:nil)
    private var target=NSPopUpButton()
    private var sliders:[String:NSSlider]=[:], values:[String:NSTextField]=[:]
    private var rows:[String:NSView]=[:]
    private var controllerView=NSView(),gamesView=NSView()
    private var gameButtons:[NSButton]=[]
    private var gameState=NSTextField(wrappingLabelWithString:"")
    private var lastWatch=Date.distantPast
    private var tabs:NSSegmentedControl?

    func show(games:Bool=false) {
        if window==nil { build() }
        loadProfile();refreshInspector()
        controllerView.isHidden=games;gamesView.isHidden = !games;tabs?.selectedSegment=games ? 1:0
        window?.makeKeyAndOrderFront(nil);NSApp.activate(ignoringOtherApps:true)
        if timer==nil { timer=Timer.scheduledTimer(withTimeInterval:0.1,repeats:true){[weak self] _ in self?.poll()} }
        studioCommand("WATCH");poll()
    }
    private func text(_ string:String,_ frame:NSRect,_ font:NSFont)->NSTextField {
        let t=NSTextField(labelWithString:string);t.frame=frame;t.font=font;return t
    }
    private func build() {
        let w=NSWindow(contentRect:NSRect(x:0,y:0,width:1130,height:790),styleMask:[.titled,.closable,.miniaturizable],backing:.buffered,defer:false)
        w.title="MK1 Revive — Controller Studio";w.appearance=NSAppearance(named:.darkAqua);w.delegate=self;w.isReleasedWhenClosed=false
        let content=NSView(frame:NSRect(x:0,y:0,width:1130,height:790));content.wantsLayer=true;content.layer?.backgroundColor=ink.cgColor;w.contentView=content
        content.addSubview(text("MK1 / STUDIO",NSRect(x:24,y:730,width:520,height:38),.systemFont(ofSize:28,weight:.bold)))
        statusLabel.frame=NSRect(x:26,y:706,width:1050,height:20);statusLabel.font = .monospacedSystemFont(ofSize:12,weight:.medium);statusLabel.textColor=cyan;content.addSubview(statusLabel)
        let tabs=NSSegmentedControl(labels:["Controller","Games"],trackingMode:.selectOne,target:self,action:#selector(tabChanged(_:)))
        tabs.frame=NSRect(x:24,y:666,width:250,height:28);tabs.selectedSegment=0;self.tabs=tabs;content.addSubview(tabs)
        controllerView.frame=NSRect(x:0,y:28,width:1130,height:625);content.addSubview(controllerView)
        gamesView.frame=controllerView.frame;content.addSubview(gamesView);gamesView.isHidden=true
        canvas.frame=NSRect(x:24,y:22,width:760,height:590);controllerView.addSubview(canvas)
        canvas.onSelect={ [weak self] kind,index in self?.selectedKind=kind;self?.selectedIndex=index;self?.refreshInspector() }
        let panel=NSView(frame:NSRect(x:806,y:22,width:300,height:590));panel.wantsLayer=true;panel.layer?.backgroundColor=NSColor(calibratedWhite:0.10,alpha:1).cgColor;panel.layer?.cornerRadius=14;controllerView.addSubview(panel)
        titleLabel.frame=NSRect(x:18,y:545,width:270,height:28);titleLabel.font = .systemFont(ofSize:21,weight:.bold);panel.addSubview(titleLabel)
        rawLabel.frame=NSRect(x:18,y:510,width:270,height:26);rawLabel.font = .monospacedDigitSystemFont(ofSize:18,weight:.medium);rawLabel.textColor=cyan;panel.addSubview(rawLabel)
        enabled.frame=NSRect(x:18,y:475,width:260,height:24);enabled.target=self;enabled.action=#selector(edited);panel.addSubview(enabled)
        override.frame=NSRect(x:18,y:443,width:270,height:24);override.target=self;override.action=#selector(edited);panel.addSubview(override)
        let specs:[(String,String,Double,Double)]=[("on","Hit-on threshold",1,4095),("off","Hit-off threshold",0,4094),("pressure","Pressure update",1,4095),("gain","Sensitivity · %",10,400)]
        for (i,spec) in specs.enumerated() {
            let row=NSView(frame:NSRect(x:18,y:385-CGFloat(i)*61,width:265,height:54));panel.addSubview(row);rows[spec.0]=row
            row.addSubview(text(spec.1,NSRect(x:0,y:30,width:195,height:20),.systemFont(ofSize:12,weight:.medium)))
            let value=text("",NSRect(x:195,y:30,width:70,height:20),.monospacedDigitSystemFont(ofSize:12,weight:.medium));value.alignment = .right;row.addSubview(value);values[spec.0]=value
            let slider=NSSlider(value:100,minValue:spec.2,maxValue:spec.3,target:self,action:#selector(edited));slider.frame=NSRect(x:0,y:5,width:265,height:22);slider.identifier=NSUserInterfaceItemIdentifier(spec.0);slider.isContinuous=true;row.addSubview(slider);sliders[spec.0]=slider
        }
        invert.frame=NSRect(x:18,y:174,width:260,height:23);invert.target=self;invert.action=#selector(edited);panel.addSubview(invert)
        panel.addSubview(text("MASCHINE destination",NSRect(x:18,y:148,width:265,height:20),.systemFont(ofSize:12,weight:.medium)))
        target.frame=NSRect(x:18,y:117,width:265,height:28);target.target=self;target.action=#selector(edited);panel.addSubview(target)
        let reset=NSButton(title:"Reset selected",target:self,action:#selector(resetSelected));reset.frame=NSRect(x:14,y:77,width:132,height:29);panel.addSubview(reset)
        let apply=NSButton(title:"Apply live",target:self,action:#selector(applyProfile));apply.frame=NSRect(x:153,y:77,width:134,height:29);apply.bezelColor=cyan;panel.addSubview(apply)
        notice.frame=NSRect(x:18,y:10,width:265,height:58);notice.font = .systemFont(ofSize:11);notice.textColor = .secondaryLabelColor;panel.addSubview(notice)
        let global=NSButton(title:"Global settings…",target:self,action:#selector(globalSettings));global.frame=NSRect(x:22,y:0,width:160,height:28);content.addSubview(global)
        buildGames()
        w.center();window=w
    }
    private func buildGames() {
        let titles=["PONG","FREEDOOM","BREAKOUT"]
        let descriptions=["Two paddles. Human input takes over instantly; each player returns to AI after 10 seconds idle.","The real Doom engine with Freedoom. Left LCD: game. Right LCD: original status bar.","A paddle, a ball and a wall of bricks. Turn encoder 1 or use pads 1 / 4. Play serves the ball."]
        for i in 0..<3 {
            let card=NSView(frame:NSRect(x:24+CGFloat(i)*363,y:200,width:346,height:360));card.wantsLayer=true;card.layer?.backgroundColor=NSColor(calibratedWhite:0.12,alpha:1).cgColor;card.layer?.cornerRadius=18;gamesView.addSubview(card)
            let symbols=["rectangle.split.2x1","scope","square.grid.3x3.fill"]
            let image=NSImageView(frame:NSRect(x:24,y:254,width:70,height:70));image.image=NSImage(systemSymbolName:symbols[i],accessibilityDescription:titles[i]);image.contentTintColor=cyan;card.addSubview(image)
            card.addSubview(text(titles[i],NSRect(x:24,y:210,width:300,height:30),.systemFont(ofSize:23,weight:.bold)))
            let desc=NSTextField(wrappingLabelWithString:descriptions[i]);desc.frame=NSRect(x:24,y:100,width:298,height:96);desc.font = .systemFont(ofSize:14);desc.textColor = .secondaryLabelColor;card.addSubview(desc)
            let button=NSButton(title:"Play \(titles[i].capitalized)",target:self,action:#selector(playGame(_:)));button.tag=i;button.frame=NSRect(x:20,y:36,width:305,height:40);button.bezelColor=cyan;card.addSubview(button);gameButtons.append(button)
        }
        gameState.frame=NSRect(x:28,y:105,width:1060,height:55);gameState.font = .systemFont(ofSize:14);gamesView.addSubview(gameState)
        let menu=NSButton(title:"Return to game selection",target:self,action:#selector(returnToMenu));menu.frame=NSRect(x:24,y:47,width:265,height:35);gamesView.addSubview(menu)
        gamesView.addSubview(text("Hardware arrows select · Play starts · Browse returns to menu",NSRect(x:305,y:53,width:760,height:22),.systemFont(ofSize:13,weight:.medium)))
    }
    private func loadProfile() {
        guard let d=try? Data(contentsOf:URL(fileURLWithPath:studioDir+"/controls.plist")),let p=try? PropertyListDecoder().decode(StudioProfile.self,from:d),p.pads.count==16,p.encoders.count==11 else {return}
        profile=p
    }
    private func refreshInspector() {
        let isPad=selectedKind=="pad",isEncoder=selectedKind=="encoder"
        titleLabel.stringValue=isPad ? "Pad \(selectedIndex+1)":(isEncoder ? knobNames[selectedIndex]:"Button \(selectedIndex)")
        override.isHidden = !isPad;invert.isHidden = !isEncoder
        for key in ["on","off","pressure"] { rows[key]?.isHidden = !isPad }
        rows["gain"]?.isHidden = !(isPad||isEncoder)
        target.removeAllItems()
        enabled.isEnabled=isPad||isEncoder;target.isEnabled=isPad||isEncoder
        if isPad {
            let p=profile.pads[selectedIndex];enabled.state=p.enabled ? .on:.off;override.state=p.override ? .on:.off
            for (k,v) in [("on",p.override ? p.on:((state["on"] as? Int) ?? 300)),("off",p.override ? p.off:((state["off"] as? Int) ?? 150)),("pressure",p.override ? p.pressure:((state["pressure"] as? Int) ?? 200)),("gain",p.gain)] {sliders[k]?.integerValue=v;values[k]?.stringValue="\(v)"}
            for key in ["on","off","pressure"] {sliders[key]?.isEnabled=p.override}
            target.addItems(withTitles:(1...16).map{"Pad \($0)"});target.selectItem(at:p.target)
        } else if isEncoder {
            let e=profile.encoders[selectedIndex];enabled.state=e.enabled ? .on:.off;invert.state=e.invert ? .on:.off
            sliders["gain"]?.integerValue=e.gain;values["gain"]?.stringValue="\(e.gain)";target.addItems(withTitles:knobNames);target.selectItem(at:e.target)
        }
        notice.stringValue="Changes apply live after saving. Destination overrides affect MASCHINE; game controls keep their physical layout."
        updateRaw()
    }
    private func updateRaw() {
        if selectedKind=="pad" {
            let p=(state["pads"] as? [Int]) ?? []
            rawLabel.stringValue="\(p.count>selectedIndex ? p[selectedIndex]:0) / 4095"
        } else if selectedKind=="encoder" {
            let d=(state["encoderDelta"] as? [Int]) ?? []
            rawLabel.stringValue=String(format:"Δ %+.4f",Double(d.count>selectedIndex ? d[selectedIndex]:0)/10000)
        } else {
            let bits=(state["buttons"] as? NSNumber)?.uint64Value ?? 0
            rawLabel.stringValue=(bits & (1<<selectedIndex)) != 0 ? "PRESSED":"Released"
        }
    }
    private func poll() {
        guard window?.isVisible == true else {return}
        if Date().timeIntervalSince(lastWatch)>1 {studioCommand("WATCH");lastWatch=Date()}
        guard let d=try? Data(contentsOf:URL(fileURLWithPath:studioDir+"/state.json")),let s=(try? JSONSerialization.jsonObject(with:d)) as? [String:Any] else {
            state=[:];canvas.state=[:];updateRaw()
            statusLabel.stringValue="Bridge offline · Start the bridge from the menu bar";gameButtons.forEach{$0.isEnabled=false};return
        }
        state=s;canvas.state=s;updateRaw()
        let usb=(s["usb"] as? Bool) ?? false,host=(s["maschine"] as? Bool) ?? false,idle=(s["idle"] as? Bool) ?? false
        statusLabel.stringValue="\(usb ? "● MK1 connected":"○ Waiting for USB")  /  \(host ? "MASCHINE connected":"Standalone controller")  /  LIVE INPUT"
        gameButtons.forEach{$0.isEnabled=usb&&idle}
        if !idle {gameState.stringValue="Games are available when MASCHINE releases the controller. Close MASCHINE to play."}
        else {
            let g=(s["game"] as? Int) ?? 0,menu=(s["menu"] as? Bool) ?? true
            let names=["Pong","Freedoom","Breakout"]
            let ai=(s["pongAI"] as? [Bool]) ?? [false,true]
            gameState.stringValue=menu ? "Hardware is in game selection. Choose a game here or use the controller arrows.":"Now playing: \(names[max(0,min(2,g))])"+(g==0&&ai.count==2 ? " · P1: \(ai[0] ? "AI":"Human") · P2: \(ai[1] ? "AI":"Human")":"")
        }
    }
    @objc private func tabChanged(_ control:NSSegmentedControl){controllerView.isHidden=control.selectedSegment==1;gamesView.isHidden=control.selectedSegment==0}
    @objc private func edited(){
        if selectedKind=="pad" {
            var p=profile.pads[selectedIndex];p.enabled=enabled.state == .on;p.override=override.state == .on
            p.on=sliders["on"]!.integerValue;p.off=sliders["off"]!.integerValue;p.pressure=sliders["pressure"]!.integerValue;p.gain=sliders["gain"]!.integerValue;p.target=max(0,target.indexOfSelectedItem);profile.pads[selectedIndex]=p
            for key in ["on","off","pressure"]{sliders[key]?.isEnabled=p.override}
        }else if selectedKind=="encoder"{
            profile.encoders[selectedIndex]=StudioEncoder(enabled:enabled.state == .on,invert:invert.state == .on,gain:sliders["gain"]!.integerValue,target:max(0,target.indexOfSelectedItem))
        }
        for (key,slider) in sliders {values[key]?.stringValue="\(slider.integerValue)"}
        notice.stringValue="Unsaved changes · Apply live to save and activate."
    }
    @objc private func resetSelected(){
        if selectedKind=="pad"{profile.pads[selectedIndex]=StudioPad(target:selectedIndex)}
        if selectedKind=="encoder"{profile.encoders[selectedIndex]=StudioEncoder(target:selectedIndex)}
        refreshInspector();notice.stringValue="Defaults restored for this control. Apply live to save."
    }
    @objc private func applyProfile(){
        guard profile.pads.allSatisfy({$0.on>$0.off}) else {notice.stringValue="Hit-off must be lower than hit-on.";return}
        do {
            try FileManager.default.createDirectory(atPath:studioDir,withIntermediateDirectories:true)
            let encoder=PropertyListEncoder();encoder.outputFormat = .xml
            let data=try encoder.encode(profile);let url=URL(fileURLWithPath:studioDir+"/controls.plist")
            try data.write(to:url,options:.atomic)
            try FileManager.default.setAttributes([.posixPermissions:0o600],ofItemAtPath:url.path)
            guard studioCommand("RELOAD") else {notice.stringValue="Saved · start bridge to apply.";return}
            notice.stringValue="Saved · applying to bridge…"
            DispatchQueue.main.asyncAfter(deadline:.now()+0.5){[weak self] in
                guard let self=self else{return}
                self.poll()
                self.notice.stringValue=(self.state["profileError"] as? Bool)==true ? "Release all pads and apply again. The bridge kept the previous profile.":"Saved · bridge notified of the new profile."
            }
        }catch{notice.stringValue="Save failed: \(error.localizedDescription)"}
    }
    @objc private func globalSettings(){SettingsWindowController.shared.show()}
    @objc private func playGame(_ button:NSButton){studioCommand("PLAY \(button.tag)");poll()}
    @objc private func returnToMenu(){studioCommand("MENU");poll()}
    func windowWillClose(_ notification:Notification){timer?.invalidate();timer=nil}
}
