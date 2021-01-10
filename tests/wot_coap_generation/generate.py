import argparse
import json
import os
import sys
from typing import List, Tuple, Type, TypedDict, IO, Any

NAMESPACE = "wot_td"
PROPERTIES_NAME = 'properties'
ACTIONS_NAME = 'actions'
EVENTS_NAME = 'events'
AFFORDANCE_TYPES = [PROPERTIES_NAME, ACTIONS_NAME, EVENTS_NAME]
AFFORDANCE_TYPE_SPECIFIERS = {
    PROPERTIES_NAME: 'prop',
    ACTIONS_NAME: 'action',
    EVENTS_NAME: 'event'
}
CURRENT_DIRECTORY = os.getcwd()
CONFIG_DIRECTORY = f"{CURRENT_DIRECTORY}/config"
THING_DESCRIPTION_DIRECTORY = f"{CONFIG_DIRECTORY}/wot_td"
RESULT_FILE = f"{CURRENT_DIRECTORY}/wot_coap_config.c"
AFFORDANCES_FILES = [".coap_affordances.json", ]
THING_FILES = [".thing.json", ]
SEPERATOR = "\n\n"
INDENT = "    "
COAP_RESOURCES_NAME = "_wot_coap_resources"
COAP_LISTENER_NAME = "_wot_coap_listener"
COAP_LINK_PARAMS_NAME = "_wot_link_params"
COAP_LINK_ENCODER_NAME = "_wot_encode_link"

DEFAULT_DEPENDENCIES = ['<stdint.h>', '<stdio.h>', '<stdlib.h>',
                        '<string.h>', '"net/gcoap.h"', '"od.h"', '"fmt.h"',
                        '"net/wot.h"', '"net/wot/coap.h"', '"net/wot/coap/config.h"', ]

COAP_LINK_ENCODER = f'''static ssize_t {COAP_LINK_ENCODER_NAME}(const coap_resource_t *resource, char *buf,
                                size_t maxlen, coap_link_encoder_ctx_t *context)
{{
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0)
    {{
        if ({COAP_LINK_PARAMS_NAME}[context->link_pos] && (strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]) < (maxlen - res)))
        {{
            if (buf)
            {{
                memcpy(buf + res, {COAP_LINK_PARAMS_NAME}[context->link_pos],
                       strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]));
            }}
            return res + strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]);
        }}
    }}

    return res;
}}'''


OPERATION_TYPES = {
    "readproperty": "FORM_OP_READ_PROPERTY",
    "writeproperty": "FORM_OP_WRITE_PROPERTY",
    "observeproperty": "FORM_OP_OBSERVE_PROPERTY",
    "unobserveproperty": "FORM_OP_UNOBSERVE_PROPERTY",
    "invokeaction": "FORM_OP_INVOKE_ACTION",
    "subscribeevent": "FORM_OP_SUBSCRIBE_EVENT",
    "unsubscribeevent": "FORM_OP_UNSUBSCRIBE_EVENT",
    "readallproperties": "FORM_OP_READ_ALL_PROPERTIES",
    "writeallproperties": "FORM_OP_WRITE_ALL_PROPERTIES",
    "readmultipleproperties": "FORM_OP_READ_MULTIPLE_PROPERTIES",
    "writemultipleproperties": "FORM_OP_WRITE_MULTIPLE_PROPERTIES"
}

CONTENT_TYPES = {
    "application/json": "CONTENT_TYPE_JSON",
    "text/plain": "CONTENT_TYPE_TEXT_PLAIN",
    "application/ld+json": "CONTENT_TYPE_JSON_LD",
    "text/comma-separated-values": "CONTENT_TYPE_CSV"
}

JSON_TYPES = {
    "object": "JSON_TYPE_OBJECT",
    "array": "JSON_TYPE_ARRAY",
    "string": "JSON_TYPE_STRING",
    "number": "JSON_TYPE_NUMBER",
    "integer": "JSON_TYPE_INTEGER",
    "boolean": "JSON_TYPE_BOOLEAN",
    "null": "JSON_TYPE_NULL"
}

ARRAY_FIELDS = {
    "minItems": "min_items",
    "maxItems": "max_items",
}

SECURITY_SCHEMA_TYPE = {
    "nosec": "SECURITY_SCHEME_NONE",
    "basic": "SECURITY_SCHEME_BASIC",
    "digest": "SECURITY_SCHEME_DIGEST",
    "apikey": "SECURITY_SCHEME_API_KEY",
    "bearer": "SECURITY_SCHEME_BEARER",
    "psk": "SECURITY_SCHEME_PSK",
    "oauth2": "SECURITY_SCHEME_OAUTH2",
}

SECURITY_SCHEMA_INFORMATION = {
    "default": "SECURITY_SCHEME_IN_DEFAULT",
    "header": "SECURITY_SCHEME_IN_HEADER",
    "query": "SECURITY_SCHEME_IN_QUERY",
    "body": "SECURITY_SCHEME_IN_BODY",
    "cookie": "SECURITY_SCHEME_IN_COOKIE",
}

CONTENT_ENCODINGS = {
    "gzip": "CONTENT_ENCODING_GZIP",
    "compress": "CONTENT_ENCODING_COMPRESS",
    "deflate": "CONTENT_ENCODING_DEFLATE",
    "identity": "CONTENT_ENCODING_IDENTITY",
    "br": "CONTENT_ENCODING_BROTLI",
}

ALLOWED_OPERATIONS_BY_TYPE = {
    PROPERTIES_NAME: ["readproperty", "writeproperty", "observeproperty", "unobserveproperty", ],
    ACTIONS_NAME: ["invokeaction", ],
    EVENTS_NAME: ["subscribeevent", "unsubscribeevent", ],
}

SECURITY_DEFINITIONS: dict = {
    "basic": {
        "scheme": "basic",
        "in": "query",
        "name": "querykey",
        "description": "Basic sec schema",
        "descriptions": {
            "en": "Basic sec schema"
        }
    },
}

used_affordance_keys: List[str] = []
header_files: List[str] = []
extern_functions: List[str] = []
resource_affordance_list: List[str] = []

ResourceDict = TypedDict(
    'ResourceDict', {'affordance_name': str, 'href': str, 'handler': str, "methods": List[str]})


def dict_raise_on_duplicates(ordered_pairs):
    """Reject duplicate keys."""
    d = {}
    for k, v in ordered_pairs:
        if k in d:
            raise ValueError("duplicate key: %r" % (k,))
        else:
            d[k] = v
    return d


def write_to_c_file(result) -> None:
    f: IO[Any] = open(RESULT_FILE, "w")
    f.write(result)
    f.close()


def validate_coap_json(coap_jsons: dict) -> None:
    # TODO: Add actual validator for (different types of) affordances
    assert coap_jsons['name'], "ERROR: name in coap_affordances.json missing"
    assert coap_jsons['url'], "ERROR: url in coap_affordances.json missing"
    assert coap_jsons['handler'], "ERROR: handler in coap_affordances.json missing"
    assert coap_jsons['method'], "ERROR: method in coap_affordances.json missing"


def get_handler_name_for_href(href: str) -> str:
    return f'wot{href.replace("/", "_")}_handler'


def get_handler_function_header(handler_name: str) -> str:
    return f'extern ssize_t {handler_name}(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);'


def validate_thing_json(thing_json: dict) -> None:
    # TODO: Expand thing validation
    assert thing_json['titles'], "ERROR: name in thing.json missing"
    assert thing_json['defaultLang'], "ERROR: name in thing.json missing"


class CStruct:
    def __init__(self, struct_type: str, struct_name: str, keywords: List[str] = [], zero_struct=False):
        self.struct_name = struct_name
        self.zero_struct = zero_struct
        self.first_line = self.__get_first_line(
            struct_type, struct_name, keywords)
        self.children: List[CStruct] = []
        self.parent = None
        if not zero_struct:
            self.elements = [self.first_line]

    def __get_first_line(self, struct_type: str, struct_name: str, keywords: List[str]) -> str:
        return f"{self.__generate_keywords(keywords)}{struct_type} {struct_name} = {{"

    def __generate_keywords(self, keywords: List[str]) -> str:
        return ' '.join(keywords) + ' ' if keywords else ''

    def __generate_zero_struct(self):
        return f"{self.first_line}0}};"

    def generate_struct(self):
        if self.zero_struct:
            return self.__generate_zero_struct()
        else:
            return f'\n{INDENT}'.join(self.elements) + "\n};"

    def generate_c_code(self) -> str:
        code = [child.generate_c_code() for child in self.children]
        code.append(self.generate_struct())
        return SEPERATOR.join(code)

    def __generate_field(self, field_name: str, field_value: str) -> str:
        return f".{field_name} = {field_value},"

    def add_reference_field(self, field_name: str, reference_name: str) -> None:
        self.add_field(field_name, f'&{reference_name}')

    def add_string_field(self, c_name: str, json_name: str, schema: dict):
        if json_name in schema:
            value: str = schema[json_name]
            assert isinstance(value, str)
            self.add_field(c_name, f'"{value}"')

    def add_boolean_field(self, c_name: str, json_name: str, schema: dict):
        if json_name in schema:
            value: bool = schema[json_name]
            assert isinstance(value, bool)
            self.add_field(c_name, get_c_boolean(value))

    def add_field(self, field_name: str, field_value: str) -> None:
        assert not self.zero_struct
        field = self.__generate_field(field_name, field_value)
        self.elements.append(field)

    def add_unordered_field(self, field: str) -> None:
        assert not self.zero_struct
        self.elements.append(f"{field},")

    def add_child(self, child: 'CStruct') -> None:
        assert not self.zero_struct
        self.children.insert(0, child)
        child.parent = self


def write_coap_resources(coap_resources: List[ResourceDict]) -> str:
    sorted_resources: List[ResourceDict] = sorted(
        coap_resources, key=lambda k: k['href'])

    result = f"const coap_resource_t {COAP_RESOURCES_NAME}[] = {{\n"
    for resource in sorted_resources:
        resource_affordance_list.append(resource['affordance_name'])
        href: str = resource["href"]
        methods: List[str] = resource['methods']
        handler_name: str = get_handler_name_for_href(href)

        result += f'    {{"{href}", '
        result += " | ".join(methods)
        result += f", {handler_name}, NULL}},\n"
    result += "};"

    return result


def generate_coap_resources(coap_jsons: list) -> List[ResourceDict]:
    coap_resources: List[ResourceDict] = []
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            for affordance_name, affordance in coap_json[affordance_type].items():
                assert_unique_affordance(affordance_name)
                used_affordance_keys.append(affordance_name)
                forms: List[dict] = affordance["forms"]
                resources: List[ResourceDict] = extract_coap_resources(
                    affordance_name, forms)
                coap_resources.extend(resources)
    return coap_resources


def assert_unique_affordance(affordance_name: str) -> None:
    assert affordance_name not in used_affordance_keys, "ERROR: Each coap affordance name has to be unique"


def extract_coap_resources(affordance_name: str, resources: List[dict]) -> List[ResourceDict]:
    hrefs: List[str] = []
    handlers: List[str] = []
    methods: List[List[str]] = []
    for resource in resources:
        href: str = resource['href']
        handler_function: str = resource['handler_function']
        method_name: str = resource['cov:methodName']
        if href not in hrefs:
            hrefs.append(href)
            handlers.append(handler_function)
            methods.append([f"COAP_{method_name}"])
        else:
            index: int = hrefs.index(href)
            assert handlers[
                index] == handler_function, f"ERROR: Different handler function for {href}"
            assert method_name not in methods[
                index], f"ERROR: Method {method_name} already used for href {href}"
            methods[index].append(f"COAP_{method_name}")

        header_file: str = resource.get("header_file", None)

        if header_file is not None and header_file not in header_files:
            header_files.append(header_file)
        elif header_file is None and handler_function not in extern_functions:
            extern_functions.append(handler_function)

    resource_list: List[ResourceDict] = []

    for index, href in enumerate(hrefs):
        dictionary = {'affordance_name': affordance_name,
                      'href': href,
                      "handler": handlers[index],
                      "methods": methods[index]
                      }  # type: ResourceDict
        resource_list.append(dictionary)

    return resource_list


def get_wot_json(file: str, directory=THING_DESCRIPTION_DIRECTORY, validation_function=None) -> dict:
    path = f'{directory}/{file}'
    try:
        f: IO[Any] = open(path)
        wot_json: dict = json.loads(f.read())
        if validation_function is not None:
            validation_function(wot_json)
    except IOError:
        print(f"ERROR reading {path} is missing")
        sys.exit(0)
    except json.decoder.JSONDecodeError:
        print(f"ERROR: json in {path} is not valid")
        sys.exit(0)
    finally:
        f.close()

    return wot_json


def parse_command_line_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Web of Things helper script')
    parser.add_argument('--board', help='Define used board')
    parser.add_argument('--saul', action='store_true',
                        help='Define if WoT TD SAUL is used')
    parser.add_argument('--security', help='Define what security is used')
    return parser.parse_args()


def assert_command_line_arguments(args: argparse.Namespace) -> None:
    assert args.board, "ERROR: Argument board has to be defined"
    assert args.security, "ERROR: Argument security has to be defined"


def generate_includes() -> str:
    dependencies = DEFAULT_DEPENDENCIES + \
        [f'"{header_file}"' for header_file in header_files]

    dependencies = [f'#include {dependency}' for dependency in dependencies]
    return "\n".join(dependencies)


def generate_extern_functions() -> str:
    functions = [get_handler_function_header(x) for x in extern_functions]
    return "\n".join(functions)


def generate_coap_listener() -> str:
    struct = CStruct("gcoap_listener_t", COAP_LISTENER_NAME, ["static"])
    struct.add_unordered_field(f"&{COAP_RESOURCES_NAME}[0]")
    struct.add_unordered_field(f"ARRAY_SIZE({COAP_RESOURCES_NAME})")
    struct.add_unordered_field(f"{COAP_LINK_ENCODER_NAME}")
    struct.add_unordered_field("NULL")
    struct.add_unordered_field("NULL")

    return struct.generate_struct()


def generate_coap_handlers(coap_resources: List[ResourceDict]) -> str:
    handlers: List[str] = []

    for resource in coap_resources:
        wot_handler: str = get_handler_name_for_href(resource['href'])
        actual_handler: str = resource['handler']

        handler = f"static ssize_t {wot_handler}(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)\n"
        handler += "{\n"
        handler += INDENT
        handler += f"return {actual_handler}(pdu, buf, len, ctx);\n"
        handler += "}"
        handlers.append(handler)

        # TODO: Add validation

    return SEPERATOR.join(handlers)


def generate_coap_link_param(coap_resource: ResourceDict) -> str:
    return "NULL,"


def generate_coap_link_params(coap_resources: List[ResourceDict]) -> str:
    struct_elements = [f"static const char *{COAP_LINK_PARAMS_NAME}[] = {{"]

    for coap_resource in coap_resources:
        struct_elements.append(generate_coap_link_param(coap_resource))

    return f'\n{INDENT}'.join(struct_elements) + "\n};"


def get_affordance_type_specifier(affordance_type: str) -> str:
    return AFFORDANCE_TYPE_SPECIFIERS[affordance_type]


def get_affordance_struct_name(affordance_name: str) -> str:
    return f'{NAMESPACE}_{affordance_name}'


def add_next_field(index: int, struct: CStruct, struct_name: str, struct_data, use_struct_index=True):
    if index + 1 < len(struct_data):
        if use_struct_index:
            next_struct = f"{struct_name}_{index + 1}"
        else:
            next_struct = struct_name
        struct.add_reference_field("next", next_struct)
    else:
        struct.add_field("next", "NULL")


def add_href(parent: CStruct,  form: dict) -> None:
    href_name = f'{parent.struct_name}_href'
    assert "href" in form, f'ERROR: "href" is mandatory in "form" elements! ({href_name})'
    parent.add_reference_field("href", href_name)
    struct = CStruct(f"{NAMESPACE}_uri_t",
                     f"{href_name}",
                     zero_struct=True)
    parent.add_child(struct)


def add_extension(parent: CStruct) -> None:
    extension_name = f'{parent.struct_name}_extension'
    parent.add_reference_field("extensions", extension_name)
    struct = CStruct(f"{NAMESPACE}_extension_t",
                     extension_name,
                     zero_struct=True)
    parent.add_child(struct)


def add_operations(parent: CStruct, form: dict, affordance_type: str) -> None:
    if "op" in form:
        op_name = f'{parent.struct_name}_op'
        parent.add_reference_field("op", f"{op_name}_0")
        operations = form["op"]
        if isinstance(operations, str):
            operations = [operations]
        for op_index, operation in enumerate(operations):
            assert operation in ALLOWED_OPERATIONS_BY_TYPE[
                affordance_type], f"Operation {operation} not allowed for affordance type {affordance_type}"
            op = CStruct(f"{NAMESPACE}_form_op_t",
                         f"{op_name}_{op_index}")
            op.add_field("op_type", OPERATION_TYPES[operation])
            add_next_field(op_index, op, op_name, operations)

            parent.add_child(op)


def get_media_type_and_parameters(media_string: str):
    media_list = [x.strip() for x in media_string.split(";")]
    media_type = media_list[0]
    parameters = [tuple(parameter.split("=")) for parameter in media_list[1:]]
    return media_type, parameters


def add_parameters(parent: CStruct, parameters: List[Tuple[str, str]]) -> None:
    parameter_name = f'{parent.struct_name}_parameter'
    for index, parameter in enumerate(parameters):
        struct_name = f'{parameter_name}_{index}'
        if index == 0:
            parent.add_reference_field("media_type_parameter", struct_name)
        key, value = parameter
        struct = CStruct(f"{NAMESPACE}_media_type_parameter_t",
                         struct_name)
        struct.add_field("key", f'"{key}"')
        struct.add_field("value", f'"{value}"')
        add_next_field(index, struct, parameter_name,
                       parameters)
        parent.add_child(struct)


def add_content_type(parent: CStruct, form: dict) -> None:
    if "contentType" in form:
        media_type, parameters = get_media_type_and_parameters(
            form["contentType"])
        struct_name = f'{parent.struct_name}_content_type'

        struct = CStruct(f'{NAMESPACE}_content_type_t',
                         struct_name)
        struct.add_field("media_type", CONTENT_TYPES[media_type])
        add_parameters(struct, parameters)

        parent.add_child(struct)
        parent.add_reference_field("content_type", struct_name)


def add_content_coding(struct: CStruct, form: dict) -> None:
    if "contentCoding" in form:
        content_coding = form["contentCoding"]
        content_coding_enum = "CONTENT_ENCODING_NONE"
        if content_coding in CONTENT_ENCODINGS:
            content_coding_enum = CONTENT_ENCODINGS[content_coding]
        struct.add_field("content_encoding", content_coding_enum)


def add_security(parent: CStruct, form: dict) -> None:
    if "security" in form:
        securities = form["security"]
        if isinstance(securities, str):
            securities = [securities]
        enumerated_securities = list(enumerate(securities))
        for index, security in enumerated_securities:
            assert security in SECURITY_DEFINITIONS
            struct_name = f'{parent.struct_name}_security_{security}'
            struct = CStruct(f"{NAMESPACE}_security_t",
                             struct_name)
            parent.add_child(struct)
            if index == 0:
                parent.add_reference_field("security", struct_name)
            struct.add_field("key", f'"{security}"')
            struct.add_reference_field(
                "value", f'{NAMESPACE}_security_schema_{security}')

            if index + 1 < len(enumerated_securities):
                next_item = enumerated_securities[index + 1][1]
                print(next_item)
                struct.add_reference_field("next",
                                           f'{parent.struct_name}_security_{next_item}')
            else:
                struct.add_field("next", "NULL")


def add_scopes(parent: CStruct, form: dict) -> None:
    if "scopes" in form:
        scopes = form["scopes"]
        if isinstance(scopes, str):
            scopes = [scopes]
        scope_name = f'{parent.struct_name}_scope'
        for index, scope in enumerate(scopes):
            struct_name = f'{scope_name}_{index}'
            struct = CStruct(f"{NAMESPACE}_auth_scopes_t",
                             struct_name)
            if index == 0:
                parent.add_reference_field("scopes",
                                           struct_name)
            struct.add_field("value", f'"{scope}"')
            add_next_field(index, struct, scope_name, scopes)
            parent.add_child(struct)


def add_response(parent: CStruct, form: dict) -> None:
    if "response" in form:
        response = form["response"]
        struct_name = f'{parent.struct_name}_response'
        struct = CStruct(f"{NAMESPACE}_expected_res_t",
                         struct_name)
        parent.add_reference_field("expected_response",
                                   struct_name)
        add_content_type(struct, response)
        parent.add_child(struct)


def add_forms(parent: CStruct, affordance_type: str,   affordance: dict) -> None:
    assert "forms" in affordance, f"ERROR: No forms defined for {parent.struct_name}"
    forms = affordance['forms']
    struct_name = f'{parent.struct_name}_form'
    for index, form in enumerate(forms):
        struct = CStruct(f'{NAMESPACE}_form_t',
                         f'{struct_name}_{index}')
        parent.add_child(struct)
        if index == 0:
            parent.add_reference_field(
                "forms", f"{struct_name}_0")
            parent.parent.parent.add_reference_field(
                "form", f"{struct_name}_0")  # FIXME: Move to href
        add_operations(struct, form, affordance_type)
        add_href(struct, form)
        add_content_type(struct, form)
        add_content_coding(struct, form)
        struct.add_string_field("sub_protocol", "subprotocol", form)
        add_security(struct, form)
        add_scopes(struct, form)
        add_response(struct, form)
        add_extension(struct)
        add_next_field(index, struct, struct_name,
                       forms)


def add_type(parent: CStruct, affordance: dict) -> None:
    if "@type" in affordance:
        struct_name = f'{parent.struct_name}_type'
        type_list: List[str] = affordance["@type"]
        if isinstance(type_list, str):
            type_list = [type_list]

        # TODO: Also use linked list for definition in struct
        parent.add_reference_field("type", f"{struct_name}_0")
        for index, type_entry in enumerate(type_list):
            struct = CStruct(f'{NAMESPACE}_type_t',
                             f'{struct_name}_{index}')
            struct.add_field("value", f'"{type_entry}"')

            add_next_field(index, struct, struct_name,
                           type_list)
            parent.add_child(struct)


def add_multi_lang(parent: CStruct, field_name: str, struct_name: str, json_key: str, affordance: dict) -> None:
    if json_key in affordance:
        complete_struct_name = f'{parent.struct_name}_{struct_name}'
        parent.add_reference_field(field_name, f"{complete_struct_name}_0")
        multi_lang_dict = affordance[json_key]
        for index, entry in enumerate(multi_lang_dict.items()):
            tag, value = entry
            struct = CStruct(f'{NAMESPACE}_multi_lang_t',
                             f'{complete_struct_name}_{index}')
            struct.add_field("tag", f'"{tag}"')
            struct.add_field("value", f'"{value}"')
            add_next_field(index, struct, complete_struct_name,
                           multi_lang_dict)
            parent.add_child(struct)


def add_interaction_affordance(parent: CStruct, affordance_type: str,  affordance: dict) -> None:
    struct_name = f'{parent.struct_name}_int'
    struct = CStruct(f"{NAMESPACE}_int_affordance_t",
                     struct_name)
    parent.add_child(struct)
    parent.add_reference_field("int_affordance", struct_name)
    add_type(struct, affordance)
    add_multi_lang(struct, "descriptions", "description",
                   "descriptions", affordance)
    add_multi_lang(struct, "titles", "title",
                   "titles", affordance)
    add_data_schema_maps(struct, "uri_variables", "uriVariables",
                         f"{parent.struct_name}_uri_variable", affordance)
    add_forms(struct, affordance_type, affordance)


def get_c_boolean(boolean: bool) -> str:
    if boolean:
        return "true"
    else:
        return "false"


def add_requirements(parent: CStruct, schema_name: str, schema: dict) -> None:
    required_properties: List[str] = get_required_properties(schema)
    if required_properties:
        for index, requirement in enumerate(required_properties):
            struct_name = f"{schema_name}_{requirement}_required"
            if index == 0:
                parent.add_reference_field('required', struct_name)
            struct = CStruct(f"{NAMESPACE}_object_required_t", struct_name)
            struct.add_field("value", f'"{requirement}"')
            parent.add_child(struct)


def add_data_schema_maps(parent: CStruct, field_name: str, json_name: str, schema_name: str, schema: dict) -> None:
    if json_name in schema:
        enumerated_properties = list(enumerate(schema[json_name].items()))
        for index, entry in enumerated_properties:
            property_name, property = entry
            data_map_name = f'{schema_name}_{property_name}_data_map'
            data_schema_name = f'{schema_name}_{property_name}_data_schema'
            if index == 0:
                parent.add_reference_field(field_name, data_map_name)

            struct = CStruct(f"{NAMESPACE}_data_schema_map_t",
                             data_map_name)
            struct.add_field("key", f'"{property_name}"')
            struct.add_reference_field("value", data_schema_name)
            if index + 1 < len(enumerated_properties):
                next_item = enumerated_properties[index + 1][1][0]
                struct.add_reference_field("next",
                                           f"{schema_name}_{next_item}_data_map")
            else:
                struct.add_field("next", "NULL")
            parent.add_child(struct)

            generate_data_schema(struct,  property, data_schema_name)


def get_required_properties(schema: dict) -> List[str]:
    required_properties = schema['required']
    if isinstance(required_properties, str):
        required_properties = [required_properties]
    for property in required_properties:
        assert property in schema['properties']

    return required_properties


def add_schema_object(parent: CStruct, schema_name: str, schema: dict) -> None:
    struct_name = f'{schema_name}_object'
    parent.add_reference_field("schema", struct_name)
    struct = CStruct(f"{NAMESPACE}_object_schema_t",
                     f"{schema_name}_object")
    add_data_schema_maps(struct, 'properties',
                         'properties', schema_name, schema)

    add_requirements(struct, schema_name, schema)

    parent.add_child(struct)


def add_json_type_schema(parent: CStruct, schema_name: str, schema: dict) -> None:
    json_type = None
    if "properties" in schema:
        json_type = "object"
    elif "type" in schema:
        json_type = schema["type"]

    if json_type is not None:
        parent.add_field("json_type", JSON_TYPES[json_type])

    struct_name = f"{schema_name}_{json_type}"

    if json_type == "object":
        add_schema_object(parent, schema_name, schema)
    elif json_type == "array":
        struct = CStruct(f"{NAMESPACE}_{json_type}_schema_t",
                         struct_name)
        for json_field_name, c_field_name in ARRAY_FIELDS.items():
            if json_field_name in schema:
                value = schema[json_field_name]
                assert isinstance(
                    value, int) and value >= 0, "Min and max values for arrays must be positive integers!"
                struct.add_field(c_field_name, str(value))
            else:
                struct.add_field(c_field_name, "NULL")
        add_data_schema_array(struct, "items", "items", schema)

        parent.add_child(struct)
        parent.add_reference_field("schema", struct_name)
    elif json_type == "number" or json_type == "integer":
        struct = CStruct(f"{NAMESPACE}_{json_type}_schema_t",
                         f"{schema_name}_{json_type}")
        for field_name in ["minimum", "maximum"]:
            if field_name in schema:
                value = schema[field_name]
                if json_type == "integer":
                    assert isinstance(
                        value, int), "Min or max values for integers must also be integers!"
                struct.add_field(field_name, str(value))
            else:
                struct.add_field(field_name, "NULL")

        parent.add_child(struct)
        parent.add_reference_field("schema", struct_name)


def add_data_schema_array(parent: CStruct, field_name: str, json_name: str, schema: dict) -> None:
    if json_name in schema:
        struct_name = f'{parent.struct_name}_{field_name}'
        data_schemas = schema[json_name]
        assert isinstance(data_schemas, list)
        for index, entry in enumerate(data_schemas):
            if index == 0:
                parent.add_reference_field(field_name, f'{struct_name}_0')
            struct = CStruct(f"{NAMESPACE}_data_schemas_t",
                             f'{struct_name}_{index}')
            struct.add_reference_field(
                "value", f'{struct_name}_{index}_schema')
            generate_data_schema(struct,
                                 entry,
                                 f'{struct_name}_{index}_schema')
            add_next_field(index, struct, struct_name, data_schemas)
            parent.add_child(struct)


def add_enumeration(parent: CStruct, schema: dict) -> None:
    # TODO: Should actually support any type as specified by WoT
    if "enum" in schema:
        enum_name = f'{parent.struct_name}_enum'
        enum_data = schema["enum"]
        assert isinstance(enum_data, list)
        for index, entry in enumerate(enum_data):
            if index == 0:
                parent.add_reference_field("enumeration", f'{enum_name}_0')
            struct = CStruct(f"{NAMESPACE}_data_enums_t",
                             f'{enum_name}_{index}')
            struct.add_field("value", f'"{entry}"')
            add_next_field(index, struct, enum_name, enum_data)
            parent.add_child(struct)


def add_data_schema_field(parent: CStruct, field_name: str, json_name: str, schema: dict):
    if json_name in schema:
        data_schema_name = f'{parent.struct_name}_{field_name}_data_schema'
        parent.add_reference_field(field_name, data_schema_name)
        generate_data_schema(parent, schema, data_schema_name)


def generate_data_schema(parent: CStruct, schema: dict, schema_name: str) -> None:
    struct = CStruct(f"{NAMESPACE}_data_schema_t",
                     f"{schema_name}")
    parent.add_child(struct)
    add_type(struct, schema)
    add_multi_lang(struct, "descriptions", "description",
                   "descriptions", schema)
    add_multi_lang(struct, "titles", "title",
                   "titles", schema)
    struct.add_string_field("constant", "const", schema)
    struct.add_string_field("unit", "unit", schema)
    struct.add_string_field("format", "format", schema)
    add_enumeration(struct, schema)
    add_data_schema_array(struct, "one_of", "oneOf", schema)
    add_json_type_schema(struct, schema_name, schema)
    struct.add_boolean_field("read_only", "readOnly", schema)
    struct.add_boolean_field("write_only", "writeOnly", schema)


def add_specific_affordance(parent: CStruct, affordance_type: str, affordance_name: str, affordance: dict) -> None:
    specifier = get_affordance_type_specifier(affordance_type)
    struct_name = f"{parent.struct_name}_{specifier}_aff"
    struct = CStruct(f'{NAMESPACE}_{specifier}_affordance_t',
                     struct_name)
    parent.add_child(struct)
    parent.add_reference_field("affordance", struct_name)
    struct.add_field("key", f'"{affordance_name}"')
    add_interaction_affordance(struct, affordance_type, affordance)
    if PROPERTIES_NAME in affordance:
        assert affordance_type == PROPERTIES_NAME
        add_data_schema_field(struct, "data_schema", "properties", affordance)
    if affordance_type == PROPERTIES_NAME:
        struct.add_boolean_field("observable", "observable", affordance)
    elif affordance_type == ACTIONS_NAME:
        struct.add_boolean_field("safe", "safe", affordance)
        struct.add_boolean_field("idempotent", "idempotent", affordance)
        add_data_schema_field(struct, "input", "input", affordance)
        add_data_schema_field(struct, "output", "output", affordance)
    elif affordance_type == EVENTS_NAME:
        add_data_schema_field(struct, "subscription",
                              "subscription", affordance)
        add_data_schema_field(struct, "data", "data", affordance)
        add_data_schema_field(struct, "cancellation",
                              "cancellation", affordance)

    struct.add_field("next", "NULL")  # TODO: Create linked list?


def generate_affordance_struct(affordance_type: str, affordance_name: str, affordance: dict) -> str:
    resource_index = resource_affordance_list.index(affordance_name)

    struct_specifier = get_affordance_type_specifier(affordance_type)
    struct_name = get_affordance_struct_name(affordance_name)
    struct = CStruct(f"{NAMESPACE}_coap_{struct_specifier}_affordance_t",
                     struct_name)
    struct.add_reference_field(
        "coap_resource", f"{COAP_RESOURCES_NAME}[{resource_index}]")  # TODO: Move to href

    add_specific_affordance(struct, affordance_type,
                            affordance_name, affordance)

    return struct.generate_c_code()


def generate_json_serialization(coap_jsons: List[dict]) -> str:
    result_elements = []
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            for affordance in coap_json[affordance_type].items():
                result_elements.append(generate_affordance_struct(affordance_type, affordance[0],
                                                                  affordance[1]))
    return SEPERATOR.join(result_elements)


def generate_affordance_entries(affordance_type: str, affordance_type_json: dict) -> str:
    result = ""
    specifier = get_affordance_type_specifier(affordance_type)
    for affordance_name in affordance_type_json:
        struct_name: str = get_affordance_struct_name(affordance_name)
        result += INDENT
        result += f'{NAMESPACE}_coap_{specifier}_add(thing, &{struct_name});\n'

    return result


def generate_init_function(coap_jsons: List[dict]) -> str:
    result = f"int {NAMESPACE}_coap_config_init({NAMESPACE}_thing_t *thing)\n"
    result += "{\n"
    result += INDENT + f"gcoap_register_listener(&{COAP_LISTENER_NAME});\n"
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            result += generate_affordance_entries(
                affordance_type, coap_json[affordance_type])

    result += INDENT + "return 0;\n"
    result += "}\n"

    return result


def add_to_result(result_element: str, result_elements: List[str]):
    if result_element:
        result_elements.append(result_element)


def assemble_results(coap_jsons: List[dict], thing_jsons: List[dict]) -> List[str]:
    coap_resources = generate_coap_resources(coap_jsons)

    result_elements: List[str] = []
    add_to_result(generate_includes(), result_elements)
    add_to_result(generate_extern_functions(), result_elements)
    add_to_result(generate_coap_handlers(coap_resources), result_elements)
    add_to_result(write_coap_resources(coap_resources), result_elements)
    add_to_result(generate_coap_link_params(coap_resources), result_elements)
    add_to_result(COAP_LINK_ENCODER, result_elements)
    add_to_result(generate_coap_listener(), result_elements)
    add_to_result(generate_json_serialization(coap_jsons), result_elements)
    add_to_result(generate_init_function(coap_jsons), result_elements)

    return result_elements


def get_result() -> str:
    coap_jsons: List[dict] = [get_wot_json(affordance_file)
                              for affordance_file in AFFORDANCES_FILES]
    thing_jsons: List[dict] = [get_wot_json(thing_file, validation_function=validate_thing_json)
                               for thing_file in THING_FILES]

    result_elements: List[str] = assemble_results(coap_jsons, thing_jsons)

    return SEPERATOR.join(result_elements)


def main() -> None:
    args = parse_command_line_arguments()
    assert_command_line_arguments(args)

    result: str = get_result()
    write_to_c_file(result)


if __name__ == '__main__':
    main()
