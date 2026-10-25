import AppKit

struct DoomWADInfo {
    let kind: String
    let commercial: Bool?
}
struct DoomWADSettings: Codable {
    var baseWAD: String?
    var patchWAD: String?
    static var directory: URL { URL(fileURLWithPath:NSHomeDirectory()+"/Library/Application Support/MK1 Revive") }
    static var file: URL { directory.appendingPathComponent("doom.plist") }
    static func load() -> Self {
        guard let data=try? Data(contentsOf:file),let value=try? PropertyListDecoder().decode(Self.self,from:data) else { return Self() }
        return value
    }
    static func inspect(_ url: URL) throws -> DoomWADInfo {
        func invalid() -> NSError { NSError(domain:"DoomWAD",code:1,userInfo:[NSLocalizedDescriptionKey:"Choose a valid classic Doom IWAD or PWAD (maximum 256 MiB). PK3/GZDoom packages are not supported."]) }
        let file=try FileHandle(forReadingFrom:url);defer {try? file.close()}
        let size=try file.seekToEnd();try file.seek(toOffset:0)
        guard size>=12,size<=256*1024*1024,let header=try file.read(upToCount:12),header.count==12 else {throw invalid()}
        let kind=String(bytes:header.prefix(4),encoding:.ascii) ?? ""
        func uint32(_ data:Data,_ at:Int)->UInt64 { (0..<4).reduce(UInt64(0)) { $0 | UInt64(data[at+$1]) << ($1*8) } }
        let count=uint32(header,4),offset=uint32(header,8)
        guard ["IWAD","PWAD"].contains(kind),count>0,count<=262144,offset>=12,offset+count*16<=size else {throw invalid()}
        try file.seek(toOffset:offset)
        guard let entries=try file.read(upToCount:Int(count*16)),entries.count==Int(count*16) else {throw invalid()}
        var commercial: Bool?
        for i in 0..<Int(count) {
            let at=i*16
            guard uint32(entries,at)+uint32(entries,at+4)<=size else {throw invalid()}
            let name=String(bytes:entries[(at+8)..<(at+16)].prefix(while:{$0 != 0}),encoding:.ascii) ?? ""
            if commercial==nil {
                if name.range(of:"^E[1-9]M[1-9]$",options:.regularExpression) != nil {commercial=false}
                else if name.range(of:"^MAP[0-9][0-9]$",options:.regularExpression) != nil && name != "MAP00" {commercial=true}
            }
        }
        if kind=="IWAD" && commercial==nil {throw invalid()}
        return DoomWADInfo(kind:kind,commercial:commercial)
    }
    func save() throws {
        try FileManager.default.createDirectory(at:Self.directory,withIntermediateDirectories:true)
        try PropertyListEncoder().encode(self).write(to:Self.file,options:.atomic)
        try FileManager.default.setAttributes([.posixPermissions:0o600],ofItemAtPath:Self.file.path)
    }
    mutating func importWAD(_ source: URL) throws {
        let info=try Self.inspect(source)
        let bundled=Bundle.main.resourceURL!.appendingPathComponent("freedoom/freedoom1.wad")
        if info.kind=="PWAD",let kind=info.commercial {
            let baseInfo=try Self.inspect(baseWAD.map{URL(fileURLWithPath:$0)} ?? bundled)
            if kind != baseInfo.commercial {throw NSError(domain:"DoomWAD",code:2,userInfo:[NSLocalizedDescriptionKey:"This add-on uses a different map format. Choose a compatible base IWAD first (Doom/Ultimate Doom or Doom II)."])}
        }
        let folder=Self.directory.appendingPathComponent("DoomWADs",isDirectory:true)
        try FileManager.default.createDirectory(at:folder,withIntermediateDirectories:true)
        let destination=folder.appendingPathComponent(UUID().uuidString+"-"+source.lastPathComponent)
        try FileManager.default.copyItem(at:source,to:destination)
        let old=self
        if info.kind=="IWAD" {baseWAD=destination.path;patchWAD=nil} else {patchWAD=destination.path}
        do {try save()} catch {self=old;try? FileManager.default.removeItem(at:destination);throw error}
        removeUnused(old)
    }
    mutating func restoreBundled() throws {let old=self;let next=Self();try next.save();self=next;removeUnused(old)}
    mutating func removePatch() throws {let old=self;var next=self;next.patchWAD=nil;try next.save();self=next;removeUnused(old)}
    private func removeUnused(_ previous: Self) {
        let folder=Self.directory.appendingPathComponent("DoomWADs",isDirectory:true).standardizedFileURL
        for path in [previous.baseWAD,previous.patchWAD].compactMap({$0}) where path != baseWAD && path != patchWAD {
            let url=URL(fileURLWithPath:path).standardizedFileURL
            if url.deletingLastPathComponent()==folder {try? FileManager.default.removeItem(at:url)}
        }
    }
    var summary: String {
        func name(_ path:String?)->String? {
            guard let path else{return nil}
            let name=URL(fileURLWithPath:path).lastPathComponent
            return name.count>37 && name[name.index(name.startIndex,offsetBy:36)]=="-" ? String(name.dropFirst(37)):name
        }
        return "Base: \(name(baseWAD) ?? "Bundled Freedoom")   ·   Add-on: \(name(patchWAD) ?? "None")"
    }
}
