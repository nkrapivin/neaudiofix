{
  "resourceType": "GMExtension",
  "resourceVersion": "1.2",
  "name": "neaudiofix_ext",
  "optionsFile": "options.json",
  "options": [],
  "exportToGame": true,
  "supportedTargets": -1,
  "extensionVersion": "1.0.0",
  "packageId": "",
  "productId": "",
  "author": "",
  "date": "2023-10-14T22:43:24.2335051+05:00",
  "license": "",
  "description": "",
  "helpfile": "",
  "iosProps": false,
  "tvosProps": false,
  "androidProps": false,
  "html5Props": false,
  "installdir": "",
  "files": [
    {"resourceType":"GMExtensionFile","resourceVersion":"1.0","name":"","filename":"neaudiofix_x64.dll","origname":"","init":"","final":"","kind":1,"uncompress":false,"functions":[
        {"resourceType":"GMExtensionFunction","resourceVersion":"1.0","name":"neaudiofix_is_present","externalName":"","kind":1,"help":"neaudiofix_is_present()","hidden":false,"returnType":2,"argCount":0,"args":[],"documentation":"",},
        {"resourceType":"GMExtensionFunction","resourceVersion":"1.0","name":"neaudiofix_enum_devices","externalName":"","kind":1,"help":"neaudiofix_enum_devices()","hidden":false,"returnType":2,"argCount":0,"args":[],"documentation":"",},
        {"resourceType":"GMExtensionFunction","resourceVersion":"1.0","name":"neaudiofix_enum_get_name","externalName":"","kind":1,"help":"neaudiofix_enum_get_name(deviceIndex)","hidden":false,"returnType":1,"argCount":0,"args":[
            2,
          ],"documentation":"",},
        {"resourceType":"GMExtensionFunction","resourceVersion":"1.0","name":"neaudiofix_enum_devices_select","externalName":"","kind":1,"help":"neaudiofix_enum_devices_select(deviceIndex)","hidden":false,"returnType":2,"argCount":0,"args":[
            2,
          ],"documentation":"",},
        {"resourceType":"GMExtensionFunction","resourceVersion":"1.0","name":"__neaudiofix_RegisterCallbacks","externalName":"RegisterCallbacks","kind":1,"help":"__neaudiofix_RegisterCallbacks(p1,p2,p3,p4)","hidden":false,"returnType":2,"argCount":0,"args":[
            1,
            1,
            1,
            1,
          ],"documentation":"",},
        {"resourceType":"GMExtensionFunction","resourceVersion":"1.0","name":"neaudiofix_periodic","externalName":"","kind":1,"help":"neaudiofix_periodic()","hidden":false,"returnType":2,"argCount":0,"args":[],"documentation":"",},
      ],"constants":[],"ProxyFiles":[],"copyToTargets":64,"usesRunnerInterface":false,"order":[],},
  ],
  "HTML5CodeInjection": "",
  "classname": "",
  "tvosclassname": null,
  "tvosdelegatename": null,
  "iosdelegatename": "",
  "androidclassname": "",
  "sourcedir": "",
  "androidsourcedir": "",
  "macsourcedir": "",
  "maccompilerflags": "",
  "tvosmaccompilerflags": "",
  "maclinkerflags": "",
  "tvosmaclinkerflags": "",
  "iosplistinject": "",
  "tvosplistinject": "",
  "androidinject": "",
  "androidmanifestinject": "",
  "androidactivityinject": "",
  "gradleinject": "",
  "androidcodeinjection": "",
  "hasConvertedCodeInjection": false,
  "ioscodeinjection": "",
  "tvoscodeinjection": "",
  "iosSystemFrameworkEntries": [],
  "tvosSystemFrameworkEntries": [],
  "iosThirdPartyFrameworkEntries": [],
  "tvosThirdPartyFrameworkEntries": [],
  "IncludedResources": [],
  "androidPermissions": [],
  "copyToTargets": 64,
  "iosCocoaPods": "",
  "tvosCocoaPods": "",
  "iosCocoaPodDependencies": "",
  "tvosCocoaPodDependencies": "",
  "parent": {
    "name": "NeAudioFix",
    "path": "folders/NeAudioFix.yy",
  },
}