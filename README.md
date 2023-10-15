# NeAudioFix

A hook for GameMaker's `ALCdevice_wasapi` class to swap between multiple audio output devices on the fly.

***Only works on the Windows 64-bit target, LTS 2022.0.1 or newer, VM or YYC.***

# Acknowledgements

`NeSampleTune` in the sample project is `bgmInvincibility.ogg` taken from the game [Sonic Time Twisted](https://github.com/overbound/SonicTimeTwisted), check it out!

`MinHook` library is available [here](https://github.com/TsudaKageyu/minhook), very amazing stuff!

# Public API

## neaudiofix_is_present

`neaudiofix_is_present()->bool`

Returns `true` if the hook is present and functioning correctly, `false` otherwise.

If the function returns `false` please do not call any other `neaudiofix` functions as they may crash the game.

## neaudiofix_enum_devices

`neaudiofix_enum_devices()->real`

Enumerates all available active WASAPI output devices and returns the total count or a negative value on error.

## neaudiofix_enum_get_name

`neaudiofix_enum_get_name(deviceIndex:real)->string`

Returns a friendly name for a device, the index must be in `[0;neaudiofix_enum_devices())` range.

An empty string is returned on failure.

## neaudiofix_enum_devices_select

`neaudiofix_enum_devices_select(deviceIndex:real)->real`

Forces GameMaker backend to a new audio device, a negative index value may be used to choose a default device.

Returns `1` on success and a negative error code on failure.

## neaudiofix_periodic

`neaudiofix_periodic()->real`

This function should be called in a Step event to automatically restart the playback thread if it got stopped due to an error.

By default GameMaker will not attempt to restart the playback thread if an audio device got lost, this function will try to recover the thread.

# GML wrapper for the enumerate function(s)

```gml
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
		if (!is_undefined(rv) && rv) { // return true to quit loop early
			break;
		}
		++idx;
	}
	
	return true;
}
```

A very simple-ish wrapper to quickly enumerate the devices, example:

```gml
var devices = [];
var okay = NeEnumerateDevices(function(index, name, count, arg) {
	arg[index] = name;
},
devices);
if (okay) {
	// `devices` should be populated with device names
	// then use neaudiofix_enum_devices_select(idx) to force a specific device,
	// or -1 to switch back to default.
}
```

