// 2 november 2017
import Cocoa
import CoreText

func fourccString(_ k: FourCharCode) -> String {
	var c: [UInt8] = [0, 0, 0, 0]
	c[0] = UInt8((k >> 24) & 0xFF)
	c[1] = UInt8((k >> 16) & 0xFF)
	c[2] = UInt8((k >> 8) & 0xFF)
	c[3] = UInt8(k & 0xFF)
	return String(bytes: c, encoding: .utf8)!
}

var weightMin: Double = 0
var weightMax: Double = 0
var weightDef: Double = 0
var weightVals: [String: Double] = [:]

let attrs: [String: String] = [
	kCTFontFamilyNameAttribute as String:		"Skia",
]
let bd = CTFontDescriptorCreateWithAttributes(attrs as CFDictionary)
let matches = CTFontDescriptorCreateMatchingFontDescriptors(bd, nil) as! [CTFontDescriptor]
let mfont = CTFontCreateWithFontDescriptor(matches[0], 0.0, nil)
let master = CTFontCopyVariationAxes(mfont) as! [NSDictionary]
master.forEach { d in
	print("axis {")
	d.forEach { k, v in
		if (k as! String) == (kCTFontVariationAxisIdentifierKey as String) {
			let c = v as! FourCharCode
			print("\t\(k) \(fourccString(c))")
		} else {
			print("\t\(k) \(v)")
		}
	}
	print("}")
	if (d[kCTFontVariationAxisNameKey] as! String) == "Weight" {
		weightMin = d[kCTFontVariationAxisMinimumValueKey] as! Double
		weightMax = d[kCTFontVariationAxisMaximumValueKey] as! Double
		weightDef = d[kCTFontVariationAxisDefaultValueKey] as! Double
	}
}
print("")
matches.forEach { d in
	let f = CTFontDescriptorCopyAttribute(d, kCTFontVariationAttribute) as! [FourCharCode: Double]
	let n = CTFontDescriptorCopyAttribute(d, kCTFontStyleNameAttribute) as! String
	print("\(n) {")
	f.forEach { k, v in
		print("\t\(fourccString(k)) \(v)")
	}
	print("}")
	weightVals[n] = weightDef
	if let v = f[2003265652] {
		weightVals[n] = v
	}
}
print("")
weightVals.forEach { k, v in
	let basicScaled = (v - weightMin) / (weightMax - weightMin)
	print("\(k) basic scaled = \(basicScaled) (OS2 \(UInt16(basicScaled * 1000)))")
	// https://www.microsoft.com/typography/otspec/otvaroverview.htm#CSN
	var opentypeScaled: Double = 0
	if v < weightDef {
		opentypeScaled = -((weightDef - v) / (weightDef - weightMin))
	} else if v > weightDef {
		opentypeScaled = (v - weightDef) / (weightMax - weightDef)
	}
	print("\(k) opentype scaled = \(opentypeScaled)")
}
print("")
print("\(String(describing: CTFontDescriptorCreateMatchingFontDescriptors(CTFontDescriptorCreateCopyWithVariation(matches[0], FourCharCode(2003265652) as CFNumber, CGFloat(weightMax)), Set([kCTFontVariationAttribute as String]) as CFSet)))")
print("")
print("\(CTFontCopyTable(mfont, CTFontTableTag(kCTFontTableAvar), []) != nil)")
