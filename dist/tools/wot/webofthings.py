# Copyright (C) 2021 Universität Bremen
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import os
import sys
import json
import copy
import requests
from urllib.parse import urlparse, urljoin
from typing import (
    IO,
    Any
)


class ThingDescription(object):

    def __init__(self, raw_thing_model, meta_data, bindings, placeholders):
        thing_model = self.replace_placeholders(
            dict(raw_thing_model), placeholders)
        setattr(self, "@context", thing_model.get("@context", []))
        setattr(self, "@type", thing_model.get("@type", []))
        self.id = thing_model.get("id")
        self.title = thing_model.get("title")
        self.titles = thing_model.get("titles", dict())
        self.description = thing_model.get("description")
        self.descriptions = thing_model.get("descriptions", dict())
        self.version = thing_model.get("version")
        self.created = thing_model.get("created")
        self.modified = thing_model.get("modified")
        self.support = thing_model.get("support")
        self.base = thing_model.get("base")
        self.properties = thing_model.get("properties", dict())
        self.actions = thing_model.get("actions", dict())
        self.events = thing_model.get("events", dict())
        self.links = thing_model.get("links", [])
        self.forms = thing_model.get("forms", [])
        self.security = thing_model.get("security", [])
        self.securityDefinitions = thing_model.get(
            "securityDefinitions", dict())

        self.insert_meta_data(meta_data)
        self.insert_bindings(bindings)
        self._set_context()
        self._set_type()
        self.validate()

    def validate(self):
        for affordance_type in ["properties", "actions", "events"]:
            self.validate_affordances(affordance_type)

        for security in self.security:
            assert security in self.securityDefinitions

    def validate_affordances(self, affordance_type):
        affordances = getattr(self, affordance_type)
        required_affordances = affordances.pop("required")
        for required_affordance in required_affordances:
            assert required_affordance in affordances

    def __iter__(self):
        for key in self.__dict__:
            yield key, getattr(self, key)

    def _set_type(self):
        at_type = getattr(self, "@type")
        at_type = ["Thing" if x == "ThingModel" else x for x in at_type]
        setattr(self, "@type", at_type)

    def _set_context(self):
        context = getattr(self, "@context")
        context = ["https://www.w3.org/2019/wot/td/v1" if x ==
                   "http://www.w3.org/ns/td" else x for x in context]
        setattr(self, "@context", context)

    @staticmethod
    def replace_placeholders(thing_model, placeholders):
        if placeholders is None:
            return thing_model
        import re
        thing_model_as_string = json.dumps(thing_model)

        for placeholder, value in placeholders.items():
            assert isinstance(placeholder, str)
            assert re.fullmatch('[A-Z0-9_]+',
                                placeholder), "Placeholders must follow the pattern \"PLACEHOLDER_IDENTIFIER\""
            assert isinstance(value, str)

            thing_model_as_string = thing_model_as_string.replace(
                "{{" + placeholder + "}}", value)

        assert "{{" not in thing_model_as_string, "Not all placeholders have been replaced!"

        return json.loads(thing_model_as_string)

    def insert_meta_data(self, meta_data):
        if meta_data is None:
            return
        if meta_data.get("@context") and isinstance(meta_data["@context"], str):
            meta_data["@context"] = [meta_data["@context"]]
        for context in meta_data.get("@context", []):
            if context == "http://www.w3.org/ns/td" or context == "https://www.w3.org/2019/wot/td/v1":
                continue
            elif context not in getattr(self, "@context"):
                getattr(self, "@context").append(context)
        if meta_data.get("@type") and isinstance(meta_data["@type"], str):
            meta_data["@type"] = [meta_data["@type"]]
        for json_ld_type in meta_data.get("@type", []):
            if json_ld_type != "ThingModel":
                getattr(self, "@type").append(json_ld_type)
        default_security = False
        if not meta_data.get("securityDefinitions"):
            if not self.securityDefinitions:
                self.securityDefinitions = {"nosec_sc": {"scheme": "nosec"}}
                self.security = ["nosec_sc"]
                default_security = True
                info = 'No security definitions found! Using "no security" as default.'
                print(f"[INFO] {info}")
        else:
            if not self.securityDefinitions:
                self.securityDefinitions = meta_data["securityDefinitions"]
            else:
                for security_definition in meta_data["securityDefinitions"]:
                    self.securityDefinitions.append(security_definition)

        securities = meta_data.get("security", [])
        if isinstance(securities, str):
            securities = [securities]
        if not default_security:
            if not self.security:
                self.security = securities
            else:
                for security in securities:
                    if security not in self.security:
                        self.security.append(security)

        for link in meta_data.get("links", []):
            if link not in self.links:
                self.links.append(link)

        # FIXME: Rework mechanism for inserting instance meta data
        for field_name in ["title", "titles", "description", "descriptions", "id", "version", "created", "modified",
                           "support"]:
            if field_name in meta_data:
                setattr(self, field_name, meta_data[field_name])

    def insert_bindings(self, bindings):
        if bindings is None:
            return
        for affordance_type in ["properties", "actions", "events"]:
            affordance_bindings = bindings.get(affordance_type, dict())
            affordances = getattr(self, affordance_type)
            for key, value in affordance_bindings.items():
                if key in affordances:
                    affordance = affordances[key]
                    assert "forms" in value
                    if "forms" not in affordance:
                        affordance["forms"] = value["forms"]
                    else:
                        for form in value["forms"]:
                            for model_form in affordance["forms"]:
                                if form["href"] == model_form["href"]:
                                    if "op" in form and "op" in model_form:
                                        if form["op"] == model_form["op"]:
                                            for k, v in form.items():
                                                model_form[k] = v
                                        elif isinstance(form["op"], str) and isinstance(model_form["op"], list):
                                            for k, v in form.items():
                                                if k == "op":
                                                    continue
                                                model_form[k] = v


class ThingModel(object):
    def __init__(self, input_json: dict, origin_url=None, origin_path=None, perform_extension=True):
        wot_json = copy.deepcopy(input_json)
        setattr(self, "@context", wot_json.get("@context", []))
        setattr(self, "@type", wot_json.get("@type", []))
        self.id = wot_json.get("id")
        self.title = wot_json.get("title")
        self.titles = wot_json.get("titles", dict())
        self.description = wot_json.get("description")
        self.descriptions = wot_json.get("descriptions", dict())
        self.version = wot_json.get("version", dict())
        self.created = wot_json.get("created")
        self.modified = wot_json.get("modified")
        self.support = wot_json.get("support")
        self.base = wot_json.get("base")
        self.properties = wot_json.get("properties", {"required": []})
        self.actions = wot_json.get("actions", {"required": []})
        self.events = wot_json.get("events", {"required": []})
        self.links = wot_json.get("links", [])
        self.forms = wot_json.get("forms", [])
        self.security = wot_json.get("security", [])
        self.securityDefinitions = wot_json.get("securityDefinitions", dict())

        self.validate()
        if perform_extension:
            self.perform_extension(origin_url, origin_path)

    def __iter__(self):
        for key in self.__dict__:
            yield key, getattr(self, key)

    @classmethod
    def get_from_url(cls, url, perform_extension=True):
        thing_model_json = requests.get(url).text
        thing_model_dict = json.loads(thing_model_json)
        prepare_wot_json(thing_model_dict)
        return ThingModel(thing_model_dict, perform_extension=perform_extension)

    @classmethod
    def get_from_file_path(cls, path, perform_extension=True):
        assert os.path.isfile(path), "Invalid path for Thing Model given!"
        try:
            file: IO[Any] = open(path)
            try:
                thing_model_dict: dict = json.loads(file.read())
                prepare_wot_json(thing_model_dict)
            except json.decoder.JSONDecodeError:
                print(f"ERROR: json in {path} is not valid")
                sys.exit(0)
            finally:
                file.close()
        except (IOError, OSError):
            print(f"ERROR reading {path} is missing")
            sys.exit(0)

        return ThingModel(thing_model_dict, perform_extension=perform_extension)

    def validate(self):
        # TODO: Move all validation of the TD to this function
        context = getattr(self, "@context")
        at_type = getattr(self, "@type")

        if isinstance(context, str):
            context = [context]
        else:
            assert isinstance(context, list)
        setattr(self, "@context", context)

        if isinstance(context, str):
            at_type = [at_type]
        else:
            assert isinstance(at_type, list)
        setattr(self, "@type", at_type)

        if isinstance(self.security, str):
            self.security = [self.security]
        else:
            assert isinstance(self.security, list)

        for affordance_type in ["properties", "actions", "events"]:
            affordances = getattr(self, affordance_type)
            if "required" in affordances:
                assert isinstance(affordances["required"], list)
            else:
                affordances["required"] = []

    def get_extension_link(self):
        for link in self.links:
            if "extends" in link.get("rel", []):
                assert "href" in link
                # Not tm+json?
                assert link.get("type") == "application/td+json"
                return link

        return None

    def extend(self, thing_model):
        self.extend_meta_data(thing_model)
        self.extend_links(thing_model)
        self.extend_security(thing_model)
        self.extend_affordances(thing_model)

    @staticmethod
    def clean_up_list_field(containing_object, key):
        assert isinstance(containing_object[key], list)
        containing_object[key] = list(set(containing_object[key]))

    def extend_affordances(self, thing_model):
        for affordance_type in ["events", "properties", "actions"]:
            for affordance_name, definition in getattr(thing_model, affordance_type).items():
                affordances = getattr(self, affordance_type)
                if affordance_name not in affordances:
                    affordances[affordance_name] = definition
                elif affordance_name == "required":
                    if "required" not in affordances:
                        affordances["required"] = []
                    for requirement in definition:
                        if requirement not in affordances["required"]:
                            affordances["required"].append(requirement)
                else:
                    for key, value in definition.items():
                        affordance = affordances[affordance_name]
                        if key not in affordance:
                            affordance[key] = value

    def extend_security(self, thing_model):
        for x in thing_model.security:
            if x not in thing_model.security:
                self.security.append(x)

        for name, definition in thing_model.securityDefinitions.items():
            if name not in self.securityDefinitions:
                self.securityDefinitions[name] = definition
            else:
                for key, value in thing_model.securityDefinitions[name].items():
                    if key not in self.securityDefinitions[key]:
                        self.securityDefinitions[key] = value

    def extend_link(self, new_link, existing_link):
        # TODO: More complex logic for links is needed for the new TD specification
        # See: https://w3c.github.io/wot-thing-description/#link
        pass

    def extend_links(self, thing_model):
        for existing_link in self.links:
            if existing_link.get("rel") == "extends":
                self.links.remove(existing_link)
        self.merge_array_fields(thing_model, "links")

    def merge_array_fields(self, thing_model, field_name):
        content = getattr(self, field_name)
        for x in getattr(thing_model, field_name):
            if x not in content:
                content.append(x)

    def extend_meta_data(self, thing_model):
        # TODO: Check if @context and @type should be extended this way
        # TODO: Check how to deal with context extensions
        for field in ["@type", "@context", "version"]:
            self.merge_array_fields(thing_model, field)

        for field in ["id", "title", "titles", "description", "descriptions",
                      "created", "modified", "support", "base"]:
            if not getattr(self, field):
                setattr(self, field, getattr(thing_model, field))

    @staticmethod
    def get_extension_thing_model(href, origin_url, origin_path):
        parsed_href = urlparse(href)

        if parsed_href.netloc:
            return ThingModel.get_from_url(href)
        elif origin_url:
            absolute_url = urljoin(origin_url, parsed_href.path)
            return ThingModel.get_from_url(absolute_url)
        elif origin_path:
            path = os.path.join(origin_path, parsed_href.path)
            path = os.path.normpath(path)
            return ThingModel.get_from_file_path(path)
        else:
            raise IOError("No valid URL or path for Thing Model given!")

    def perform_extension(self, origin_url, origin_path):
        extension_link = self.get_extension_link()

        if extension_link:
            href = extension_link["href"]
            thing_model = self.get_extension_thing_model(
                href, origin_url, origin_path)
            self.extend(thing_model)

    def generate_thing_description(self, meta_data, bindings, placeholders):
        return ThingDescription(self, meta_data, bindings, placeholders)

    def __str__(self):
        return str(self.__class__) + ": " + str(self.__dict__)


def turn_string_field_to_list(wot_json, field_name):
    field = wot_json.get(field_name, [])
    if isinstance(field, str):
        wot_json[field_name] = [field]
    else:
        assert isinstance(field, list)


def prepare_wot_json(wot_json):
    turn_string_field_to_list(wot_json, "@context")
    turn_string_field_to_list(wot_json, "@type")
    turn_string_field_to_list(wot_json, "security")
