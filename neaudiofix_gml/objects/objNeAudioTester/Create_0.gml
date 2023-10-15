/// @description Вставьте описание здесь
// Вы можете записать свой код в этом редакторе



soundInst = audio_play_sound(NeSampleTune, 2, true);



curDevices = [];
reenum = function() {
	NeEnumerateDevices(function(idx, name, cnt, arg) {
		arg[idx] = name;
	},
	curDevices);
};

reenum();




