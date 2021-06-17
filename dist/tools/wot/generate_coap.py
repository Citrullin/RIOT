#!/usr/bin/env python3

# Copyright (C) 2021 Universität Bremen
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import argparse
import json
import os
import sys

from urllib.parse import urlparse
from typing import (
    List,
    IO,
    Any,
)
from wot_structs import (
    ThingInitFunction,
    CStruct,
    HandlerFunction,
)
from webofthings import (
    ThingDescription,
    ThingModel,
    prepare_wot_json,
)


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
SEPARATOR = "\n\n"
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
        if ({COAP_LINK_PARAMS_NAME}[context->link_pos]
            && (strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]) < (maxlen - res)))
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


DEFAULT_OPERATIONS_BY_TYPE = {
    PROPERTIES_NAME: ["readproperty", "writeproperty", ],
    ACTIONS_NAME: ["invokeaction", ],
    EVENTS_NAME: ["subscribeevent", ],
}

DEFAULT_COAP_METHOD_BY_OPERATION = {
    "readproperty": "COAP_GET",
    "writeproperty": "COAP_PUT",
    "observeproperty": "COAP_GET",
    "unobserveproperty": "COAP_GET",
    "invokeaction": "COAP_POST",
    "subscribeevent": "COAP_GET",
    "unsubscribeevent": "COAP_GET",
    "readallproperties": "COAP_GET",
    "writeallproperties": "COAP_PUT",
    "readmultipleproperties": "COAP_GET",
    "writemultipleproperties": "COAP_PUT",
}

used_affordance_keys: List[str] = []
header_files: List[str] = []
extern_functions: List[str] = []
resource_affordance_list: List[str] = []


def write_to_c_file(result, result_file) -> None:
    f: IO[Any] = open(f'{result_file}', "w")
    f.write(result)
    f.close()


def generate_handler_name(href: str) -> str:
    # TODO: Replace with regex
    cleaned_href = href.replace("/", "_").replace("-", "_")
    return f'wot{cleaned_href}_handler'


def get_handler_function_header(handler_name: str) -> str:
    return f'extern ssize_t {handler_name}(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);'


def write_coap_resources(coap_resources: List[dict]) -> str:
    sorted_resources: List[dict] = sorted(
        coap_resources, key=lambda k: k['href'])

    result = f"const coap_resource_t {COAP_RESOURCES_NAME}[] = {{\n{INDENT}"
    result_elements: List[str] = []
    for resource in sorted_resources:
        resource_affordance_list.append(resource['affordance_name'])
        href: str = resource["href"]
        methods: List[str] = resource['methods']
        handler_name: str = generate_handler_name(href)

        resource_list: List[str] = []
        resource_list.append("{" + f'"{href}"')
        resource_list.append(" | ".join(methods))
        resource_list.append(handler_name)
        resource_list.append("NULL")

        result_elements.append(", ".join(resource_list))
    result += f"}},\n{INDENT}".join(result_elements)
    result += "}\n};"

    return result


def generate_coap_resources(thing) -> List[dict]:
    coap_resources: List[dict] = []

    for affordance_type in AFFORDANCE_TYPES:
        for affordance_name, affordance in thing[affordance_type].items():
            assert affordance_name not in used_affordance_keys, "ERROR: Each coap affordance name has to be unique"
            used_affordance_keys.append(affordance_name)
            forms: List[dict] = affordance["forms"]
            resources: List[dict] = extract_coap_resources(
                affordance_name, affordance_type, forms)
            coap_resources.extend(resources)

    if "forms" in thing:
        thing_resources: List[dict] = extract_coap_resources(
            "thing", "thing", thing["forms"])
        coap_resources.extend(thing_resources)

    return coap_resources


def extract_coap_resources(affordance_name: str, affordance_type: str, resources: List[dict]) -> List[dict]:
    hrefs: List[str] = []
    handlers: List[str] = []
    methods: List[List[str]] = []
    for resource in resources:
        href: str = resource['href']
        handler_function: str = resource['riot_os:handler_function']
        if affordance_type == "thing":
            assert resource["op"]
        if "op" in resource:
            ops = resource["op"]
            if isinstance(ops, str):
                ops = [ops]
        elif affordance_type == "type":
            raise ValueError(f"No op defined for thing resource {href}!")
        else:
            ops = DEFAULT_OPERATIONS_BY_TYPE[affordance_type]
        op_methods = [DEFAULT_COAP_METHOD_BY_OPERATION[x] for x in ops]
        if href not in hrefs:
            hrefs.append(href)
            handlers.append(handler_function)
            methods.append(op_methods)
        else:
            index: int = hrefs.index(href)
            assert handlers[
                index] == handler_function, f"ERROR: Different handler function for {href}"
            for method_name in op_methods:
                assert method_name not in methods[
                    index], f"ERROR: Method {method_name} already used for href {href}"
            methods[index].extend(op_methods)

        header_file = resource.get("riot_os:header_file")

        if header_file and header_file not in header_files:
            header_files.append(header_file)
        elif header_file is None and handler_function not in extern_functions:
            extern_functions.append(handler_function)

    resource_list: List[dict] = []

    for index, href in enumerate(hrefs):
        dictionary = {'affordance_name': affordance_name,
                      'href': href,
                      "handler": handlers[index],
                      "methods": methods[index]
                      }  # type: dict
        resource_list.append(dictionary)

    return resource_list


def get_wot_json(app_path: str, json_path: str) -> dict:
    path = f'{app_path}/{json_path}'

    try:
        file: IO[Any] = open(path)
        try:
            wot_json: dict = json.loads(file.read())
            prepare_wot_json(wot_json)
        except json.decoder.JSONDecodeError:
            print(f"ERROR: json in {path} is not valid")
            sys.exit(0)
        finally:
            file.close()
    except (IOError, OSError):
        print(f"ERROR reading {path} is missing")
        sys.exit(0)

    return wot_json


def parse_command_line_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Web of Things helper script')
    parser.add_argument('--appdir', help='Define directory of app')
    parser.add_argument('--thing_models',
                        help="Thing Models (in JSON format) which serve as the basis of the Thing Description",
                        nargs='+')
    parser.add_argument('--meta_data_path',
                        nargs='?',
                        help="JSON file with user defined meta data")
    parser.add_argument('--placeholders_path',
                        nargs='?',
                        help="JSON file with placeholders replacements")
    parser.add_argument('--bindings_path',
                        nargs='?',
                        help="JSON file with bindings")
    parser.add_argument('--output_path',
                        help="The path to the output file")
    return parser.parse_args()


def assert_command_line_arguments(args: argparse.Namespace) -> None:
    assert args.meta_data_path, "ERROR: No instance information defined!"


def generate_includes() -> str:
    dependencies = DEFAULT_DEPENDENCIES + \
        [f'"{header_file}"' for header_file in header_files]

    dependencies = [f'#include {dependency}' for dependency in dependencies]
    return "\n".join(dependencies)


def generate_extern_functions() -> str:
    functions = [get_handler_function_header(x) for x in extern_functions]
    return "\n".join(functions)


def generate_coap_listener() -> str:
    struct = CStruct("gcoap_listener_t", None, keywords=[
                     "static"], use_namespace=False, struct_name=COAP_LISTENER_NAME)
    struct.add_unordered_field(f"&{COAP_RESOURCES_NAME}[0]")
    struct.add_unordered_field(f"ARRAY_SIZE({COAP_RESOURCES_NAME})")
    struct.add_unordered_field(f"{COAP_LINK_ENCODER_NAME}")
    struct.add_unordered_field("NULL")
    struct.add_unordered_field("NULL")

    return struct.generate_struct()


def generate_coap_handlers(coap_resources: List[dict]) -> str:
    handlers: List[str] = []

    for resource in coap_resources:
        wot_handler: str = generate_handler_name(resource['href'])
        actual_handler: str = resource['handler']

        handler_function = HandlerFunction.create(wot_handler, actual_handler)

        handlers.append(handler_function.generate_c_code())

        # TODO: Add validation

    return SEPARATOR.join(handlers)


def generate_coap_link_param(coap_resource: dict) -> str:
    # TODO: Add link parameters (in sorted order!)
    return "NULL,"


def generate_coap_link_params(coap_resources: List[dict]) -> str:
    struct_elements = [f"static const char *{COAP_LINK_PARAMS_NAME}[] = {{"]

    for coap_resource in coap_resources:
        struct_elements.append(generate_coap_link_param(coap_resource))

    return f'\n{INDENT}'.join(struct_elements) + "\n};"


def generate_init_function() -> str:
    separator = f"\n{INDENT}"

    result = [f"int {NAMESPACE}_coap_config_init({NAMESPACE}_thing_t *thing)\n{{", "(void) thing;",
              f"gcoap_register_listener(&{COAP_LISTENER_NAME});", "return 0;\n}\n"]

    return separator.join(result)


def add_to_result(result_element: str, result_elements: List[str]):
    if result_element:
        result_elements.append(result_element)


def assemble_results(thing) -> List[str]:
    """

    :param thing:
    :return:
    """
    coap_resources = generate_coap_resources(thing)

    result_elements: List[str] = []
    add_to_result(generate_includes(), result_elements)
    add_to_result(generate_extern_functions(), result_elements)
    add_to_result(generate_coap_handlers(coap_resources), result_elements)
    add_to_result(write_coap_resources(coap_resources), result_elements)
    add_to_result(generate_coap_link_params(coap_resources), result_elements)
    add_to_result(COAP_LINK_ENCODER, result_elements)
    add_to_result(generate_coap_listener(), result_elements)
    add_to_result(ThingInitFunction(thing).generate_c_code(), result_elements)
    add_to_result(generate_init_function(), result_elements)

    return result_elements


def get_thing_model(app_dir_path, thing_model_path):
    parsed_path = urlparse(thing_model_path)

    if parsed_path.netloc:
        thing_model = ThingModel.get_from_url(thing_model_path)
    else:
        path = os.path.join(app_dir_path, thing_model_path)
        path = os.path.normpath(path)
        thing_model = ThingModel.get_from_file_path(path)

    return thing_model


def get_result(app_dir_path, thing_model_paths, meta_data_path, bindings_path, placeholder_path) -> str:
    thing_models = [get_thing_model(app_dir_path, path)
                    for path in thing_model_paths]
    thing_model = thing_models[0]
    for x in range(1, len(thing_models)):
        thing_model.extend(thing_models[x])

    placeholders = None
    if placeholder_path:
        placeholders = get_wot_json(app_dir_path, placeholder_path)

    meta_data = None
    if meta_data_path:
        meta_data = get_wot_json(app_dir_path, meta_data_path)
        meta_data = ThingDescription.replace_placeholders(
            meta_data, placeholders)

    bindings = None
    if bindings_path:
        bindings = get_wot_json(app_dir_path, bindings_path)
        bindings = ThingDescription.replace_placeholders(
            bindings, placeholders)

    thing_model = thing_model.generate_thing_description(
        meta_data, bindings, placeholders)
    thing_model = dict(thing_model)

    result_elements: List[str] = assemble_results(thing_model)

    return SEPARATOR.join(result_elements)


def main_func() -> None:
    args = parse_command_line_arguments()
    assert_command_line_arguments(args)

    result: str = get_result(
        args.appdir, args.thing_models, args.meta_data_path, args.bindings_path, args.placeholders_path)

    with open(args.output_path, 'w') as writer:
        writer.write(result)


if __name__ == '__main__':
    main_func()
