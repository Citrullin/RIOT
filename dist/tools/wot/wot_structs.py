# Copyright (C) 2021 Universität Bremen
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import warnings
from dateutil import parser

from typing import (
    List,
    Tuple,
    Dict,
    Any,
    Optional,
    cast,
)

NAMESPACE = "wot_td"
THING_NAME = "thing"
NEW_LINE = "\n"
SEPARATOR = NEW_LINE * 2
INDENT = "    "

SECURITY_SCHEME_TYPE = {
    "nosec": "SECURITY_SCHEME_NONE",
    "basic": "SECURITY_SCHEME_BASIC",
    "digest": "SECURITY_SCHEME_DIGEST",
    "apikey": "SECURITY_SCHEME_API_KEY",
    "bearer": "SECURITY_SCHEME_BEARER",
    "psk": "SECURITY_SCHEME_PSK",
    "oauth2": "SECURITY_SCHEME_OAUTH2",
}
"""Map of WoT security scheme identifiers to the
values of the C enum type `wot_td_sec_scheme_type_t`.

See
[here]({https://www.w3.org/TR/wot-thing-description}/#td-vocab-scheme--SecurityScheme)
for more information.
"""

SECURITY_SCHEMA_INFORMATION_LOCATIONS = {
    "default": "SECURITY_SCHEME_IN_DEFAULT",
    "header": "SECURITY_SCHEME_IN_HEADER",
    "query": "SECURITY_SCHEME_IN_QUERY",
    "body": "SECURITY_SCHEME_IN_BODY",
    "cookie": "SECURITY_SCHEME_IN_COOKIE",
}
"""Map of WoT security scheme authentication locations
to the values of the C enum type `wot_td_sec_scheme_in_t`.

See
[here](https://www.w3.org/TR/wot-thing-description/#td-vocab-in--DigestSecurityScheme)
for more information.
"""

SECURITY_SCHEMA_QOP = {
    "auth": "SECURITY_DIGEST_QOP_AUTH",
    "auth-int": "SECURITY_DIGEST_QOP_AUTH_INT",
}
"""Map of quality of protection values for the digest security
scheme to the values of the C enum type `wot_td_digest_qop_t`.

See
[here](https://www.w3.org/TR/wot-thing-description/#td-vocab-qop--DigestSecurityScheme)
for more information.
"""

MEDIA_TYPES = {
    "none": "MEDIA_TYPE_NONE",
    "application/json": "MEDIA_TYPE_JSON",
    "text/plain": "MEDIA_TYPE_TEXT_PLAIN",
    "application/ld+json": "MEDIA_TYPE_JSON_LD",
    "text/comma-separated-values": "MEDIA_TYPE_CSV"
}
"""
Map of common media types to the values of the C enum type `wot_td_media_type_t`.
"""

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
"""
Map of all possible operation types for interaction affordances
to the values of the C enum type `wot_td_form_op_type_t`.

See
[here](https://www.w3.org/TR/wot-thing-description/#td-vocab-op--Form)
for more information.
"""

CONTENT_ENCODINGS = {
    "none": "CONTENT_ENCODING_NONE",
    "gzip": "CONTENT_ENCODING_GZIP",
    "compress": "CONTENT_ENCODING_COMPRESS",
    "deflate": "CONTENT_ENCODING_DEFLATE",
    "identity": "CONTENT_ENCODING_IDENTITY",
    "br": "CONTENT_ENCODING_BROTLI",
}
"""
Map of common content coding values to the values of the C enum type
`wot_td_content_encoding_type_t`.

See
[here](https://www.w3.org/TR/wot-thing-description/#td-vocab-contentCoding--Form)
for more information.
"""

JSON_TYPES = {
    "none": "JSON_TYPE_NONE",
    "object": "JSON_TYPE_OBJECT",
    "array": "JSON_TYPE_ARRAY",
    "string": "JSON_TYPE_STRING",
    "number": "JSON_TYPE_NUMBER",
    "integer": "JSON_TYPE_INTEGER",
    "boolean": "JSON_TYPE_BOOLEAN",
    "null": "JSON_TYPE_NULL"
}
"""
Map of possible JSON data types to the values of the C enum type
`wot_td_json_type_t`.

See
[here](https://www.w3.org/TR/wot-thing-description/#td-vocab-type--DataSchema)
for more information.
"""


class CObject(object):
    """Base class for WoT C data structures.

    This class serves as the basis for generating the needed C data
    structures that will flashed onto the MCU. Every C struct (except for
    the Thing itself) has a parent and an arbitrary number of children that
    can be converted into C code recursively using the `generate_c_code`
    method.

    Args:
        struct_type (str): The type of the struct. Will be used for generating
            the struct`s type name.
        parent: The struct's parent.
        keywords (list, optional): Optional keywords that can be inserted
            before the struct (e. g. `static`).
        data (dict, optional): The input data used for parsing. Either the
            complete or parts of the Thing Description. If omitted, the `data`
            of the struct's `parent` will be reused.
        use_namespace: If `True`, the namespace `wot_td_` will be added to
            the resulting variable name as a prefix.
        struct_name (str, optional): The name of the struct.

    Attributes:
        struct_type (str): The type of the struct.
        struct_name (int): The name of the struct.

    """

    def __init__(self, struct_type: str, parent: Optional["CObject"], keywords=None, data=None,
                 use_namespace=True, struct_name=None):
        if keywords is None:
            keywords = []
        self._keywords = keywords
        self._struct_type = struct_type
        self.parent = parent
        self.data: dict = parent.data if parent and data is None else data
        self.use_namespace = use_namespace
        self._struct_name = struct_name

        self.children: List[CObject] = []
        self.variables: List[Tuple[str, str, str]] = []
        self.elements: List[str] = []
        self.options: dict = parent.options if parent else {}

    @property
    def struct_type(self) -> str:
        """
        The type of the struct.
        """
        struct_type = self._struct_type
        if self.use_namespace:
            struct_type = f'{NAMESPACE}_{struct_type}'
        return struct_type

    @property
    def _first_line(self) -> str:
        raise NotImplementedError

    @property
    def _last_line(self) -> str:
        raise NotImplementedError

    @property
    def keywords(self) -> list:
        """A list of C keywords defined for this object. """

        return self._keywords

    @property
    def struct_name(self) -> str:
        """The name of the struct."""
        return self._get_struct_name()

    @struct_name.setter
    def struct_name(self, struct_name) -> None:
        self._set_struct_name(struct_name)

    def _get_struct_name(self) -> str:
        return self._struct_name

    def _set_struct_name(self, struct_name) -> None:
        self._struct_name = struct_name

    @property
    def securityDefinitionMap(self) -> dict:
        """
        The security definitions identified while parsing theThing Description.
        """
        return self.parent.securityDefinitionMap if self.parent else {}

    def generate_struct(self) -> str:
        lines = [self._first_line] + self.elements
        return f'{NEW_LINE}{INDENT}'.join(lines) + self._last_line

    @staticmethod
    def _variable_to_string(pointer) -> str:
        variable_type, variable_name, variable_value = pointer
        if variable_type == "char":
            variable_name += "[]"
        return f'{variable_type} {variable_name} = {variable_value};'

    def generate_c_code(self) -> str:
        code = [child.generate_c_code() for child in self.children]
        code.append(self.generate_struct())
        return SEPARATOR.join(code)

    def _generate_field(self, field_name: str, field_value: str) -> str:
        raise NotImplementedError

    def add_reference_field(self, field_name: str, reference_name: str) -> None:
        self._add_field(field_name, f'&{reference_name}')

    def add_string_field(self, key: str, value) -> None:
        if value is not None:
            self._add_field(key, f'"{value}"')

    def add_plain_field(self, key: str, value: Optional[Any]) -> None:
        if value is not None:
            self._add_field(key, str(value))

    def add_boolean_field(self, key: str, value: Optional[bool]) -> None:
        if value is None:
            return
        boolean = "false"
        if value:
            boolean = "true"
        self._add_field(key, boolean)

    def _add_field(self, field_name: str, field_value: str, insert_at_front=False) -> None:
        field = self._generate_field(field_name, field_value)
        if insert_at_front:
            self.elements.insert(0, field)
        else:
            self.elements.append(field)

    def add_unordered_field(self, field: str) -> None:
        self.elements.append(f"{field},")

    def add_child(self, child: "CObject", add_at_back=False) -> None:
        if add_at_back:
            self.children.append(child)
        else:
            self.children.insert(0, child)
        child.parent = self


class CVariable(CObject):

    def __init__(self, parent: CObject, type: str, name: str, value: Any,
                 keywords: Optional[List[str]]) -> None:

        self.type_name = type
        self.field_name = name
        self.value = value
        self.options = parent.options

        super().__init__(self.type_name, parent, use_namespace=False, keywords=keywords)

        parent.add_reference_field(self.field_name, self.struct_name)
        parent.add_child(self)

    @classmethod
    def create(cls, parent: CObject, type: str, field_name: str, value: Optional[Any], keywords=None) -> None:
        if value:
            cls(parent, type, field_name, value, keywords)

    def _get_struct_name(self) -> str:
        assert self.parent
        return f'{self.parent.struct_name}_{self.field_name}'

    def generate_c_code(self):
        keywords = " ".join(self.keywords)
        return f'{keywords} {self.type_name} {self.struct_name} = {self.value};'


class CFunction(CObject):

    def __init__(self, return_type: str, function_name: str, parameters=None, keywords=None, data=None,
                 use_namespace=True):
        if parameters is None:
            parameters = []
        self.parameters: List[Tuple[str, str]] = parameters

        super().__init__(return_type, None, data=data, struct_name=function_name, keywords=keywords, use_namespace=use_namespace)

    def add_function_line(self, line: str) -> None:
        raise NotImplementedError

    @property
    def struct_type(self) -> str:
        return self._struct_type

    def _get_struct_name(self) -> str:
        struct_name = self._struct_name
        if self.use_namespace:
            struct_name = f'{NAMESPACE}_{struct_name}'

        return struct_name

    @property
    def _first_line(self) -> str:
        parameters = ", ".join([f'{x} {y}' for (x, y) in self.parameters])
        return f"{self.struct_type} {self.struct_name}({parameters}){{"

    @property
    def _last_line(self) -> str:
        return f"{SEPARATOR + INDENT}return 0;{NEW_LINE}}}"


class ThingInitFunction(CFunction):

    def __init__(self, thing_description) -> None:
        super().__init__('int', "wot_td_config_init", data=thing_description, parameters=[("wot_td_thing_t", "*thing")],
        use_namespace=False)
        self._generate_fields()

    def _generate_fields(self) -> None:
        ContextStruct.parse(self)
        TypeStruct.parse(self)

        UriStruct.parse(self, "id")
        UriStruct.parse(self, "support")

        MultiLangStruct.parse(self, "titles")
        MultiLangStruct.parse(self, "descriptions")
        VersionStruct.parse(self)
        DateTimeStruct.parse(self, "created")
        DateTimeStruct.parse(self, "modified")
        SecurityStruct.parse(self)
        LinkStruct.parse(self)
        FormStruct.parse(self)

        PropertyAffordanceStruct.parse(self)
        ActionAffordanceStruct.parse(self)
        EventAffordanceStruct.parse(self)

        SecurityDefinitionsStruct.parse(self)

    @property
    def securityDefinitionMap(self) -> dict:
        securityDefinitionsMap = {}
        for index, key in enumerate(self.data["securityDefinitions"]):
            securityDefinitionsMap[key] = f'{self.struct_name}_security_definition_{index}'

        return securityDefinitionsMap

    @property
    def _last_line(self) -> str:
        return f"{SEPARATOR + INDENT}return 0;{NEW_LINE}}}"

    def _generate_field(self, field_name: str, field_value: str) -> str:
        return f"thing->{field_name} = {field_value};"


class HandlerFunction(CFunction):

    def __init__(self, return_type: str, wot_handler: str, actual_handler: str, parameters: List[Tuple[str, str]], keywords: List[str]):
        # self.handler_name = function_name
        super().__init__(return_type, wot_handler, parameters=parameters, keywords=keywords, use_namespace=False)

        self.elements.append(f'return {actual_handler}(pdu, buf, len, ctx);')

    @classmethod
    def create(cls, wot_handler: str, actual_handler: str) -> "HandlerFunction":
        keywords = ["static"]
        return_type = "ssize_t"
        parameters = [("coap_pkt_t", "*pdu"), ("uint8_t", "*buf"), ("size_t", "len"), ("void", "*ctx")]
        return cls(return_type, wot_handler, actual_handler, parameters, keywords)

    @property
    def _last_line(self) -> str:
        return "\n}"


class CStruct(CObject):

    def _generate_field(self, field_name: str, field_value: str) -> str:
        return f".{field_name} = {field_value},"

    @property
    def _first_line(self) -> str:
        return f"{' '.join(self.keywords)} {self.struct_type} {self.struct_name} = {{"

    @property
    def _last_line(self) -> str:
        return f"{NEW_LINE}}};"


class FieldStruct(CStruct):
    """
    Base class for structs that represent fields of the Thing Description.

    """

    def __init__(self, parent: CObject, type_name: str, field_name: str, index=0, data=None, ref_name=None,
                 use_namespace=True) -> None:
        self.type_name = type_name
        self.field_name = field_name
        self.index = index
        self.ref_name = self.field_name if ref_name is None else ref_name
        self.options = parent.options

        super().__init__(f'{self.type_name}_t', parent,
                         data=data, use_namespace=use_namespace)

        if index == 0:
            parent.add_reference_field(self.ref_name, self.struct_name)
        parent.add_child(self)

        self._generate_fields()

    def _generate_fields(self) -> None:
        """
        Converts the relevant fields of the Thing Description into C struct fields.
        """
        pass

    def _get_struct_name(self) -> str:
        """
        Generates the name of the struct.

        Overrides the getter function of the parent class to append
        the `field_name` of this struct to the `struct_name` of its
        parent.

        Returns:
            The name of the struct.
        """
        assert self.parent
        return f'{self.parent.struct_name}_{self.field_name}'


class UriStruct(FieldStruct):

    @classmethod
    def parse(cls, parent: CObject, field_name: str, data=None) -> None:
        uri = parent.data.get(field_name)
        if uri:
            cls(parent, "uri", field_name, data=data)

    @staticmethod
    def split_uri(uri, separator) -> Tuple[str, str]:
        schema, value = tuple(uri.split(separator, 1))
        schema += separator
        return schema, value

    def _generate_fields(self) -> None:
        uri = self.data[self.field_name]
        schema = None

        if "://" in uri:
            schema, value = self.split_uri(uri, "://")
        elif ":" in uri:
            schema, value = self.split_uri(uri, ":")
        else:
            value = uri

        # TODO: Save used schemas for reuse
        if schema:
            self.add_string_field("schema", schema)
        self.add_string_field("value", value)


class DateTimeStruct(FieldStruct):

    @classmethod
    def parse(cls, parent: CObject, field_name: str) -> None:
        if parent.data.get(field_name):
            cls(parent, "date_time", field_name)

    @staticmethod
    def _get_utc_offset(parsed_date) -> Optional[int]:
        utcoffset = parsed_date.utcoffset()
        if utcoffset:
            return utcoffset.seconds // 60

        return None

    def _generate_fields(self) -> None:
        date = self.data[self.field_name]
        parsed_date = parser.parse(date)
        utc_offset = self._get_utc_offset(parsed_date)

        self.add_plain_field("year", parsed_date.year)
        self.add_plain_field("month", parsed_date.month)
        self.add_plain_field("day", parsed_date.day)
        self.add_plain_field("hour", parsed_date.hour)
        self.add_plain_field("minute", parsed_date.minute)
        self.add_plain_field("second", parsed_date.second)
        self.add_plain_field("timezone_offset", utc_offset)


class VersionStruct(FieldStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("version"):
            cls(parent, "version_info", "version")

    def _generate_fields(self) -> None:
        version_info = self.data["version"]
        self.add_string_field("instance", version_info.get("instance"))


class LinkedListStruct(FieldStruct):
    def __init__(self, parent: CObject, type_name: str, field_name: str, index=0, data=None, ref_name=None,
                 use_namespace=True) -> None:
        super().__init__(parent, type_name, field_name,
                         index=index, data=data, ref_name=ref_name, use_namespace=use_namespace)
        self._add_next_field()

    @property
    def _iterable_data(self) -> list:
        field = self.data.get(self.field_name, [])
        if isinstance(field, str):
            field = [field]
        else:
            assert isinstance(field, list)
        return field

    @classmethod
    def _get_next_struct(cls, old_struct) -> "LinkedListStruct":
        return cls(old_struct.parent, old_struct.type_name, old_struct.field_name, index=old_struct.index + 1,
                   use_namespace=old_struct.use_namespace)

    def _add_next_field(self) -> None:
        data_ = self._iterable_data
        if self.index + 1 < len(data_):
            next_struct = self._get_next_struct(self)
            self.add_reference_field("next", next_struct.struct_name)
        else:
            self.add_plain_field("next", "NULL")

    def _get_struct_name(self) -> str:
        assert self.parent
        return f'{self.parent.struct_name}_{self.type_name}_{self.index}'


class TypeStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if "@type" in parent.data:
            cls(parent, "type", "@type", ref_name="type")

    def _generate_fields(self) -> None:
        self.add_string_field("value", self._iterable_data[self.index])


class MultiLangStruct(LinkedListStruct):

    @staticmethod
    def get_singular_key(field_name) -> str:
        """Removes the trailing 's' from the specified field name.
           Expects the field name to have an 's' at its end."""
        assert field_name[-1] == "s"
        return field_name[0:-1]

    @classmethod
    def parse(cls, parent: CObject, field_name: str) -> None:
        singular_key = cls.get_singular_key(field_name)
        if singular_key in parent.data or field_name in parent.data:
            cls(parent, "multi_lang", field_name)

    @property
    def _iterable_data(self) -> List[Tuple[str, str]]:
        singular_key: str = self.get_singular_key(self.field_name)
        default_value: Optional[str] = self.data.get(singular_key)

        multi_lang_dict: Dict[str, str] = self.data.get(self.field_name, {})
        default_language = self.options.get("default_language", "en")

        # TODO: This method has to be reworked.
        if multi_lang_dict:
            if default_language in multi_lang_dict:
                if default_value and default_value != multi_lang_dict[default_language]:
                    warning = f'No {singular_key} found for language "{default_language}". ' \
                              f'Using the default {singular_key} "{default_value}" instead.'
                    warnings.warn(warning)
            elif default_value:
                warning = f'No {singular_key} found for language "{default_language}". ' \
                          f'Using the default {singular_key} "{default_value}"" instead.'
                warnings.warn(warning)
                multi_lang_dict[default_language] = default_value
            else:
                error_message = f'No {singular_key} for language "{default_language}"' \
                                f'and no default {singular_key} defined.'
                raise ValueError(error_message)
        elif default_value:
            multi_lang_dict[default_language] = default_value
        else:
            raise ValueError

        return list(multi_lang_dict.items())

    def _get_struct_name(self) -> str:
        assert self.parent
        return f'{self.parent.struct_name}_{self.field_name}_{self.index}'

    def _generate_fields(self) -> None:
        tag, value = self._iterable_data[self.index]
        self.add_string_field("tag", tag)
        self.add_string_field("value", value)


class ContextStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        assert "@context" in parent.data
        obj = cls(parent, "json_ld_context", "@context",
                  ref_name="context", use_namespace=False)
        obj.set_default_language(parent)

    def set_default_language(self, parent) -> None:
        contexts = self._iterable_data
        for key, value in contexts:
            if key == "@language":
                parent.options["default_language"] = value
                parent.add_string_field("default_language_tag", value)

    @staticmethod
    def _filter_context(context_data: List[Tuple[Optional[str], str]]) -> List[Tuple[Optional[str], str]]:
        output_list = []
        language_found = False
        filter_keywords = ["https://www.w3.org/2019/wot/td/v1"]
        for key, value in context_data:
            if key == "@language":
                language_found = True
            elif key in filter_keywords:
                continue
            output_list.append((key, value))

        if not language_found:
            output_list.append(("@language", "en"))

        return output_list

    @property
    def _iterable_data(self) -> List[Tuple[Optional[str], str]]:
        context_data: List[Tuple[Optional[str], str]] = []
        for x in self.data[self.field_name]:
            if isinstance(x, str):
                context_data.append((None, x))
            elif isinstance(x, dict):
                for key, value in x.items():
                    assert isinstance(key, str)
                    assert isinstance(value, str)
                    context_data.append((key, value))

        return self._filter_context(context_data)

    def _generate_fields(self) -> None:
        type_list = self._iterable_data
        key, value = type_list[self.index]
        if key:
            self.add_string_field("key", key)
        self.add_string_field("value", value)


class LinkStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("links"):
            cls(parent, "links", "links")

    def _generate_fields(self) -> None:
        self.data = self._iterable_data[self.index]

        self.add_string_field("rel", self.data.get("rel"))

        # TODO: More media types should be supported
        media_type = MEDIA_TYPES.get(self.data.get("type", "none"))
        self.add_plain_field("type", media_type)

        UriStruct.parse(self, "href")
        UriStruct.parse(self, "anchor")


class FormStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("forms"):
            cls(parent, "form", "forms")

    def _generate_fields(self) -> None:
        self.data = self._iterable_data[self.index]

        self.add_string_field("sub_protocol", self.data.get("subprotocol"))

        contentCoding = CONTENT_ENCODINGS.get(
            self.data.get("contentCoding", "none"))
        self.add_plain_field("content_encoding", contentCoding)

        ContentTypeStruct.parse(self)
        UriStruct.parse(self, "href")
        SecurityStruct.parse(self)
        OperationTypeStruct.parse(self)
        ExpectedResponseStruct.parse(self)
        # TODO: Add extension Struct

        # TODO: Reuse scopes defined in OAuthSecuritySchemeStruct
        OAuthScopeStruct.parse(self)

    @property
    def _iterable_data(self) -> List[dict]:
        assert self.parent
        return self.parent.data["forms"]


class OAuthScopeStruct(LinkedListStruct):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("scopes"):
            cls(parent, "auth_scopes", "scopes")

    def _generate_fields(self) -> None:
        scopes = self._iterable_data
        self.add_string_field("value", scopes[self.index])


class ExpectedResponseStruct(LinkedListStruct):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("response"):
            cls(parent, "expected_res", "response", ref_name="expected_response")

    def _generate_fields(self) -> None:
        self.data = self.data["response"]

        ContentTypeStruct.parse(self)


class ContentTypeStruct(FieldStruct):

    def _get_media_type_and_parameters(self, media_string: str) -> Tuple[str, List[Tuple[str, str]]]:
        # TODO: Replace with appropriate parser library
        media_list = [x.strip() for x in media_string.split(";")]
        media_type = media_list[0]
        parameters = [self._get_parameter(parameter)
                      for parameter in media_list[1:]]
        return media_type, parameters

    def _get_parameter(self, parameter_string: str) -> Tuple[str, str]:
        split_string = parameter_string.split("=")
        return split_string[0], split_string[1]

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("contentType"):
            cls(parent, "content_type", "contentType")

    def _generate_fields(self) -> None:
        media_type, parameters = self._get_media_type_and_parameters(
            self.data["contentType"])
        self.add_plain_field("media_type", MEDIA_TYPES[media_type])
        ParametersStruct.parse(self, parameters)


class ParametersStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject, data: List[Tuple[str, str]]) -> None:
        if data:
            cls(parent, "media_type_parameter",
                "media_type_parameter", data=data)

    @property
    def _iterable_data(self) -> List[Tuple[str, str]]:
        return cast(List[Tuple[str, str]], self.data)

    def _generate_fields(self) -> None:
        data: List[Tuple[str, str]] = self._iterable_data
        key, value = data[self.index]
        self.add_string_field("key", key)
        self.add_string_field("value", value)


class OperationTypeStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("op"):
            cls(parent, "form_op", "op")

    def _generate_fields(self) -> None:
        type_list = self._iterable_data
        operation = OPERATION_TYPES[type_list[self.index]]
        self.add_plain_field("op_type", operation)


class SecurityStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("security"):
            cls(parent, "security", "security")

    def _generate_fields(self) -> None:
        definition_name = self._iterable_data[self.index]
        for key, definition in self.securityDefinitionMap.items():
            if key == definition_name:
                self.add_reference_field("definition", definition)
                return


class SecurityDefinitionsStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("securityDefinitions"):
            cls(parent, "security_definition",
                "securityDefinitions", ref_name="security_def")

    @property
    def _iterable_data(self) -> List[Tuple[str, dict]]:
        return list(self.data["securityDefinitions"].items())

    def _generate_fields(self) -> None:
        key, value = self._iterable_data[self.index]
        self.add_string_field("key", key)
        SecuritySchemeStruct.parse(self, value)


class SecuritySchemeStruct(FieldStruct):
    @classmethod
    def parse(cls, parent: CObject, data: dict) -> None:
        cls(parent, "sec_scheme", "value", data=data)

    def _generate_fields(self) -> None:
        scheme_type: str = self.data["scheme"]
        self.add_plain_field("scheme_type", SECURITY_SCHEME_TYPE[scheme_type])

        TypeStruct.parse(self)
        MultiLangStruct.parse(self, "descriptions")
        UriStruct.parse(self, "proxy")
        BasicSecuritySchemeStruct.parse(self)
        DigestSecuritySchemeStruct.parse(self)
        APIKeySchemeStruct.parse(self)
        PSKSecuritySchemeStruct.parse(self)
        OAuthSecuritySchemeStruct.parse(self)


class SecuritySchemeWithInAndName(FieldStruct):
    def _generate_fields(self) -> None:
        self.add_string_field("name", self.data.get("name"))

        in_key = self.data.get("in", "default")
        in_value = SECURITY_SCHEMA_INFORMATION_LOCATIONS[in_key]
        self.add_plain_field("in", in_value)


class BasicSecuritySchemeStruct(SecuritySchemeWithInAndName):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data["scheme"] == "basic":
            cls(parent, "basic_sec_scheme", "scheme")


class DigestSecuritySchemeStruct(SecuritySchemeWithInAndName):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data["scheme"] == "digest":
            cls(parent, "digest_sec_scheme", "scheme")

    def _generate_fields(self) -> None:
        super()._generate_fields()
        qop_value: Optional[str] = self.data.get("qop")
        if qop_value:
            self.add_plain_field("qop", SECURITY_SCHEMA_QOP.get(qop_value))


class APIKeySchemeStruct(SecuritySchemeWithInAndName):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data["scheme"] == "apikey":
            cls(parent, "api_key_sec_scheme", "scheme")


class BearerSecuritySchemeStruct(SecuritySchemeWithInAndName):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data["scheme"] == "bearer":
            cls(parent, "bearer_sec_scheme", "scheme")

    def _generate_fields(self) -> None:
        super()._generate_fields()
        for field_name in ["alg", "format"]:
            if self.data.get(field_name):
                self.add_string_field(field_name, self.data[field_name])


class PSKSecuritySchemeStruct(SecuritySchemeWithInAndName):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data["scheme"] == "psk":
            cls(parent, "psk_sec_scheme", "scheme")

    def _generate_fields(self) -> None:
        self.add_string_field("identity", self.data.get("identity"))


class OAuthSecuritySchemeStruct(SecuritySchemeWithInAndName):
    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data["scheme"] == "oauth2":
            cls(parent, "oauth2_sec_scheme", "scheme")

    def _generate_fields(self) -> None:
        self.add_string_field("identity", self.data.get("identity"))

        UriStruct.parse(self, "authorization")
        UriStruct.parse(self, "token")
        UriStruct.parse(self, "refresh")
        OAuthScopeStruct.parse(self)


class PropertyAffordanceStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("properties"):
            cls(parent, "prop_affordance", "properties")

    @property
    def _iterable_data(self) -> List[Tuple[str, dict]]:
        return list(self.data["properties"].items())

    def _generate_fields(self) -> None:
        key, value = self._iterable_data[self.index]

        self.add_string_field("key", key)
        self.add_boolean_field("observable", value.get("observable"))

        InteractionAffordanceStruct.parse(self, data=value)
        DataSchemaStruct.parse(self, "data_schema", data=value)


class ActionAffordanceStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("actions"):
            cls(parent, "action_affordance", "actions")

    @property
    def _iterable_data(self) -> List[Tuple[str, dict]]:
        return list(self.data["actions"].items())

    def _generate_fields(self) -> None:
        key, value = self._iterable_data[self.index]

        self.add_string_field("key", key)

        for field_name in ["safe", "idempotent"]:
            self.add_boolean_field(field_name, value.get(field_name))

        for field_name in ["input", "output"]:
            # TODO: Could probably be refactored
            data_schema = value.get(field_name)
            if data_schema:
                DataSchemaStruct.parse(self, field_name, data=data_schema)
        InteractionAffordanceStruct.parse(self, data=value)


class EventAffordanceStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("events"):
            cls(parent, "event_affordance", "events")

    @property
    def _iterable_data(self) -> List[Tuple[str, dict]]:
        return list(self.data["events"].items())

    def _generate_fields(self) -> None:
        data = self._iterable_data
        key, value = data[self.index]

        self.add_string_field("key", key)

        for field_name in ["subscription", "data", "cancellation"]:
            # TODO: Could probably be refactored
            data_schema = value.get(field_name)
            if data_schema:
                DataSchemaStruct.parse(self, field_name, data=data_schema)

        InteractionAffordanceStruct.parse(self, data=value)


class InteractionAffordanceStruct(FieldStruct):

    @classmethod
    def parse(cls, parent: CObject, data: dict) -> None:
        cls(parent, "int_affordance", "int_affordance", data=data)

    def _generate_fields(self) -> None:
        TypeStruct.parse(self)
        MultiLangStruct.parse(self, "titles")
        MultiLangStruct.parse(self, "descriptions")
        FormStruct.parse(self)
        DataSchemaMapStruct.parse(self, "uriVariables")


class DataSchemaMapStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject, field_name: str, data=None) -> None:
        if parent.data.get(field_name):
            cls(parent, "data_schema_map", field_name, data=data)

    @property
    def _iterable_data(self) -> List[Tuple[str, dict]]:
        return list(self.data[self.field_name].items())

    def _generate_fields(self) -> None:
        key, value = self._iterable_data[self.index]
        self.add_string_field("key", key)
        DataSchemaStruct.parse(self, "value", data=value)


class DataSchemaStruct(FieldStruct):

    @classmethod
    def parse(cls, parent: CObject, field_name: str, ref_name=None, data=None) -> None:
        cls(parent, "data_schema", field_name, ref_name=ref_name, data=data)

    def _generate_fields(self) -> None:
        TypeStruct.parse(self)
        MultiLangStruct.parse(self, "titles")
        MultiLangStruct.parse(self, "descriptions")
        self.add_plain_field(
            "json_type", JSON_TYPES[self.data.get("type", "none")])

        for field_name in ["const", "unit", "format"]:
            self.add_string_field(field_name, self.data.get(field_name))

        for ref_name, field_name in [("read_only", "readOnly"), ("write_only", "writeOnly")]:
            self.add_boolean_field(ref_name, self.data.get(field_name))

        EnumStruct.parse(self)
        DataSchemaArrayStruct.parse(self, "oneOf", ref_name="one_of")

        NumberSchemaStruct.parse(self)
        IntegerSchemaStruct.parse(self)
        ArraySchemaStruct.parse(self)
        ObjectSchemaStruct.parse(self)


class EnumStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("enum"):
            cls(parent, "data_enums", "enum", ref_name="enumeration")

    @property
    def _iterable_data(self) -> list:
        return self.data["enum"]

    def _generate_fields(self) -> None:
        self.add_string_field("value", self._iterable_data[self.index])


class DataSchemaArrayStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject, field_name: str, ref_name=None) -> None:
        if parent.data.get(field_name):
            cls(parent, "data_schemas", field_name, ref_name=ref_name)

    @property
    def _iterable_data(self) -> List[dict]:
        items = self.data[self.field_name]
        if isinstance(items, dict):
            items = [items]
        return items

    def _generate_fields(self) -> None:
        value = self._iterable_data[self.index]
        DataSchemaStruct.parse(self, "items", data=value, ref_name="value")


class NumericSchemaStruct(FieldStruct):

    def _generate_fields(self) -> None:
        for field_name in ["minimum", "maximum"]:
            self.add_plain_field(field_name, self.data.get(field_name))


class NumberSchemaStruct(NumericSchemaStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("type") == "number":
            cls(parent, "number_schema", "schema")


class IntegerSchemaStruct(NumericSchemaStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("type") == "integer":
            cls(parent, "integer_schema", "schema")


class ArraySchemaStruct(FieldStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("type") == "array":
            cls(parent, "array_schema", "schema")

    def _generate_fields(self) -> None:
        DataSchemaArrayStruct.parse(self, "items")

        for ref_name, field_name in [("min_items", "minItems"), ("max_items", "maxItems")]:
            CVariable.create(self, "uint32_t", ref_name,
                             self.data.get(field_name), keywords=["const"])


class ObjectSchemaStruct(FieldStruct):

    @classmethod
    def parse(cls, parent: CObject, data=None) -> None:
        if parent.data.get("type") == "object":
            cls(parent, "object_schema", "schema")

    def _generate_fields(self) -> None:
        DataSchemaMapStruct.parse(self, "properties")
        ObjectRequiredStruct.parse(self)


class ObjectRequiredStruct(LinkedListStruct):

    @classmethod
    def parse(cls, parent: CObject) -> None:
        if parent.data.get("required"):
            cls(parent, "object_required", "required")

    def _generate_fields(self) -> None:
        self.add_string_field("value", self._iterable_data[self.index])
