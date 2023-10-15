/// @description Вставьте описание здесь
// Вы можете записать свой код в этом редакторе


draw_set_font(fntNeAudioWithCyr);
var drawx = x, drawy = y;
var msg = "NeAudioFix MMDevices list:\nPress E to re-enumerate (something GM can't do!)";
draw_text(drawx, drawy, msg); drawy += string_height(msg) + 2;

if (keyboard_check_pressed(ord("E"))) {
	reenum();
}

for (var idx = 0, len = array_length(curDevices); idx < len; ++idx) {
	var devName = curDevices[@ idx];
	draw_text(drawx, drawy, "[" + string(idx) + "]: " + devName);
	drawy += string_height(devName) + 2;
	
	if (keyboard_check_pressed(ord("0") + idx)) {
		show_debug_message("Calling device select for " + string(idx));
		neaudiofix_enum_devices_select(idx);
		// ^^^ THIS RESETS THE AL CONTEXT!!!! 
	}
}


