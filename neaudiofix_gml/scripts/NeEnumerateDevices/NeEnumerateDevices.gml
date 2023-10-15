function NeEnumerateDevices(enumCallback, callbackArgOpt = undefined) {
	static isFixPresent = neaudiofix_is_present();
	if (!isFixPresent) {
		// DLL was not loaded into RAM
		return false;
	}
	
	var deviceCount = neaudiofix_enum_devices();
	if (deviceCount <= 0) {
		// failed or nothing to enumerate
		return false;
	}
	
	var idx = 0;
	repeat (deviceCount) {
		var deviceName = neaudiofix_enum_get_name(idx);
		var rv = enumCallback(idx, deviceName, deviceCount, callbackArgOpt);
		if (!is_undefined(rv) && rv) {
			break;
		}
		++idx;
	}
	
	return true;
}

