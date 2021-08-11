The JSON Catalogue Parser is used to parse a JSON file that contains *"catalogue information"*.
The original reason to introduce this kind of JSON file, is to collect information about the graphical resource files used by the HMI.

---
The catalogue JSON file is expected to be structured in the following way (pseudo-BNF);

<br/>< catalogue > ::= < files >
<br/>< files > ::= **"files":** < files_list>
<br/>< files_list > ::= < file > | < file > < files_list >
<br/>< file > ::= < name > < checksum > < height > < width > [ < optional_file_information > ]
<br/>< name > ::= **"name":** < string >
<br/>< checksum > ::= **"checksum":** < byte_array_as_string >
<br/>< height > ::= **"height":** < integer >
<br/>< width > ::= **"width":** < integer >

NOTE! The strings marked in bold are keywords that the implementation is searching for. This means that the JSON file must have those keys, in order for the parsing to work.

---
One example of such a JSON command file could be;

    {
        "version": "40.1_Main-App-P25_release-16_build-176",
        "files":
        [
            {
                "name": "logoAutocheck.png",
                "checksum": "55ee43a700c897cc2e7dd0905a6866ac",
                "height": 100,
                "width": 150
            },
            {
                "name": "logo.png",
                "checksum": "3f6e62b5895f19474dc64c3ca8d458e9",
                "height": 120,
                "width": 170
            },
        ]
    }

---
The user of the JSON Catalogue Parser can (after the JSON file has been correctly parsed) get information such as;

- number of files
- detailed information of a single file
