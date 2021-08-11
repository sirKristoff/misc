The JSON Command Parser is used to parse a JSON file that contains *"commands"*.
The original reason to introduce this kind of JSON file, is that it is a way to collect information that can be used to configure a mower.

---
The command JSON file is expected to be structured in the following way (pseudo-BNF);

<br/>< file > ::= < nodes >
<br/>< nodes > ::= **"nodes":** < node_list>
<br/>< node_list > ::= < node > | < node > < node_list >
<br/>< node > ::= < name > < type > < commands > [ < optional_node_information > ]
<br/>< commands > ::= **"commands":** < command_list>
<br/>< command_list > ::= < command > | < command > < command_list >
<br/>< command > ::= < msg_type > < sub_cmd > < payload > [ < expected_rsp > ] [ < optional_command_information > ]
<br/>< name > ::= **"name":** < string >
<br/>< type > ::= **"type":** < integer >
<br/>< msg_type > ::= **"msgType":** < integer_as_string >
<br/>< sub_cmd > ::= **"subCmd":** < integer_as_string >
<br/>< payload > ::= **"payload":** < byte_array_as_string >
<br/>< expected_rsp > ::= **"expectedRsp":** < byte_array_as_string >

NOTE! The strings marked in bold are keywords that the implementation is searching for. This means that the JSON file must have those keys, in order for the parsing to work.

---
One example of such a JSON command file could be;

    {
        "articleNumber": "XXXXXXX-XXX",
        "deviceType": 20,
        "nodes":
        [
            {
                "name": "Main",
                "type": 40,
                "tifDefinitionFile": "40.1_Main-App-P25_master_build-1298",
                "commands":
                [
                    {
                        "desc": "System.SetLocalHmiAvailable(available:1)",
                        "msgType": "4698",
                        "subCmd": "18",
                        "payload": "01"
                    },
                    {
                        "desc": "System.SetModel(deviceType:20,deviceVariant:0)",
                        "msgType": "4698",
                        "subCmd": "11",
                        "payload": "1400"
                    },
                    {
                        "desc": "System.SetModelName(modelName:435X)",
                        "msgType": "4698",
                        "subCmd": "17",
                        "payload": "343335580000000000000000000000000000000000"
                    },
                    {
                        "desc": "System.SetConfigVersionString(configVersion:to-be-replaced)",
                        "msgType": "4698",
                        "subCmd": "13",
                        "payload": "746F2D62652D7265706C616365640000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                    },
                    {
                        "desc": "System.SetUserMowerNameAsAsciiString(name:435X)",
                        "msgType": "4698",
                        "subCmd": "6",
                        "payload": "343335580000000000000000000000000000000000"
                    },
                    {
                        "desc": "LoopSystem.SetNbrAvailableGuides(nbrGuides:2)",
                        "msgType": "4462",
                        "subCmd": "23",
                        "payload": "02"
                    },
                    {
                        "desc": "TheftProtection.SetStopProtectionEnabled(StopProtectionEnabled:1)",
                        "msgType": "4736",
                        "subCmd": "2",
                        "payload": "01"
                    }
                ]
            },
            {
                "name": "MoreMain-2",
                "type": 40,
                "tifDefinitionFile": "40.1_Main-App-P25_master_build-1298",
                "commands":
                [
                    {
                        "desc": "System.SetConfigVersionString(configVersion:to-be-replaced)",
                        "msgType": "4698",
                        "subCmd": "13",
                        "payload": "746F2D62652D7265706C616365640000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                    }
                ]
            }
        ]
    }

---
The user of the JSON Command Parser can (after the JSON file has been correctly parsed) get information such as;

- number of nodes
- number of commands for a node
- detailed information of a single command


A single command will then contain enough information for an user of the command, to execute it in the system. This means one knows where the command should be sent (the node), which command (message type and sub command) and its payload.
