#!env python3
# -*- coding: utf-8 -*-
import re
import os
import argparse
import fileinput
import typing


class ClassDecl:
    def __init__(self, name):
        self._name = name
        self._methods = {}
        self._getters = {}
        self._setters = {}

    def get_name(self):
        return self._name

    def get_methods(self):
        return self._methods

    def get_getters(self):
        return self._getters

    def get_setters(self):
        return self._setters

    def add_method(self, name, native_func):
        if name in self._methods:
            raise RuntimeError(f'Method {name} already defined')
        if name in self._getters:
            raise RuntimeError(f'Identifier {name} already defined in getters')
        if name in self._setters:
            raise RuntimeError(f'Identifier {name} already defined in setters')
        self._methods[name] = (name, native_func)

    def add_getter(self, name, native_func):
        if name in self._methods:
            raise RuntimeError(f'Method {name} already defined')
        if name in self._getters:
            raise RuntimeError(f'Identifier {name} already defined in getters')
        self._getters[name] = (name, native_func)

    def add_setter(self, name, native_func):
        if name in self._methods:
            raise RuntimeError(f'Method {name} already defined')
        if name in self._setters:
            raise RuntimeError(f'Identifier {name} already defined in setters')
        self._setters[name] = (name, native_func)


class EnumDecl:
    def __init__(self, name, native_enum_name):
        self._name = name
        self._native_enum_name = native_enum_name
        self._fields = {}

    def get_name(self):
        return self._name

    def get_native_enum_name(self):
        return self._native_enum_name

    def get_fields(self):
        return self._fields

    def add_field(self, name, native_identifier):
        if name in self._fields:
            raise RuntimeError(f'Enum field {name} already defined')
        self._fields[name] = (name, native_identifier)


class ModuleDecl:
    def __init__(self, name, native_class_name, is_global):
        self._name = name
        self._native_class_name = native_class_name
        self._methods = {}
        self._enums = {}
        self._is_global = is_global

    def get_name(self):
        return self._name

    def get_native_class_name(self):
        return self._native_class_name

    def is_global(self):
        return self._is_global

    def get_methods(self):
        return self._methods

    def get_enums(self):
        return self._enums

    def add_method(self, name, native_func):
        if name in self._methods:
            raise RuntimeError(f'Method {name} already defined')
        if name in self._enums:
            raise RuntimeError(f'Identifier {name} already taken by an enum')
        self._methods[name] = (name, native_func)

    def add_enum(self, e: EnumDecl):
        if e.get_name() in self._enums:
            raise RuntimeError(f'Enum {e.get_name()} already defined')
        if e.get_name() in self._methods:
            raise RuntimeError(f'Identifier {e.get_name()} already taken by a method')
        self._enums[e.get_name()] = e


def _parse_native_method_name(line):
    stack = []
    maybe = []

    # 方法名左侧的括号一定是配对的（简化处理，不考虑比较运算符和'->'的情况）
    for i in range(0, len(line) + 1):
        ch = '\0' if i >= len(line) else line[i]
        if ch == '(' or ch == '<':
            if ch == '(' and len(stack) == 0:
                maybe.append(i)
            stack.append(ch)
        elif ch == ')' or ch == '>':
            expected = '(' if ch == ')' else '<'
            if len(stack) > 0 and stack[len(stack) - 1] == expected:
                stack.pop(len(stack) - 1)
            else:
                raise RuntimeError(f'Unexpected symbol "{ch}"')

    # 逐个检查可能的点
    state = 0
    for index in maybe:
        for i in range(index - 1, -2, -1):
            ch = '\0' if i < 0 else line[i]
            if state == 0:
                if ch == '\t' or ch == ' ':
                    continue
                elif ('a' <= ch <= 'z') or ('A' <= ch <= 'Z') or ch == '_':
                    state = 1
                elif '0' <= ch <= '9':
                    state = 2
                else:
                    state = -1
            elif state == 1:
                if ('a' <= ch <= 'z') or ('A' <= ch <= 'Z') or ch == '_':
                    continue
                elif '0' <= ch <= '9':
                    state = 2
                else:
                    return line[i + 1 : index]
            elif state == 2:
                if ('a' <= ch <= 'z') or ('A' <= ch <= 'Z') or ch == '_':
                    state = 1
                elif '0' <= ch <= '9':
                    continue
                else:
                    state = -1
            if state == -1:
                break
    raise RuntimeError('Cannot parse native method name')


PARSE_STATE_DEFAULT = 0
PARSE_STATE_CLASS_LOOK_FOR_CLASS_NAME = 10
PARSE_STATE_CLASS_LOOK_FOR_METHOD = 11
PARSE_STATE_CLASS_PARSE_METHOD_NAME = 12
PARSE_STATE_CLASS_PARSE_GETTER_NAME = 13
PARSE_STATE_CLASS_PARSE_SETTER_NAME = 14
PARSE_STATE_MODULE_LOOK_FOR_CLASS_NAME = 20
PARSE_STATE_MODULE_LOOK_FOR_METHOD = 21
PARSE_STATE_MODULE_PARSE_METHOD_NAME = 22
PARSE_STATE_MODULE_LOOK_FOR_ENUM_NAME = 23
PARSE_STATE_MODULE_LOOK_FOR_ENUM_FIELD = 24
PARSE_STATE_MODULE_PARSE_ENUM_FIELD = 25


class HeaderParser:
    REGEX_STRIP = re.compile(r'^\s*(.*?)\s*(//.*)?$')
    REGEX_CLASS_HINT = re.compile(r'^LSTG_CLASS(\(.*?\))?$')
    REGEX_CLASS_NAME_MATCH = re.compile(r'^(class|struct)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*')
    REGEX_CLASS_NAME_ALIAS_MATCH = re.compile(r'^(using)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=')
    REGEX_CLASS_METHOD_HINT = re.compile(r'LSTG_METHOD(\(\s*([a-zA-Z_][a-zA-Z0-9_]*)?\s*\))?$')
    REGEX_CLASS_GETTER_HINT = re.compile(r'LSTG_GETTER(\(\s*([a-zA-Z_][a-zA-Z0-9_]*)?\s*\))?$')
    REGEX_CLASS_SETTER_HINT = re.compile(r'LSTG_SETTER(\(\s*([a-zA-Z_][a-zA-Z0-9_]*)?\s*\))?$')
    REGEX_MODULE_HINT = re.compile(r'LSTG_MODULE(\(\s*([a-zA-Z_][a-zA-Z0-9_]*)?\s*(,\s*([a-zA-Z0-9_]+))?\))?$')
    REGEX_MODULE_ENUM_HINT = re.compile(r'LSTG_ENUM(\(\s*([a-zA-Z_][a-zA-Z0-9_]*)?\s*\))?$')
    REGEX_ENUM_NAME_MATCH = re.compile(r'^(enum\s+class)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*')
    REGEX_MODULE_ENUM_FIELD_HINT = re.compile(r'LSTG_FIELD(\(\s*([a-zA-Z_][a-zA-Z0-9_]*)?\s*\))?$')
    REGEX_ENUM_FIELD_MATCH = re.compile(r'^,?\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*')

    def __init__(self):
        self._state = PARSE_STATE_DEFAULT
        self._classes = {}  # type: typing.Dict[str, ClassDecl]
        self._modules = []  # type: typing.List[ModuleDecl]
        self._current_class = None  # type: typing.Optional[ClassDecl]
        self._current_module = None  # type: typing.Optional[ModuleDecl]
        self._current_enum = None  # type: typing.Optional[EnumDecl]
        self._current_hint_name = None
        self._current_flag = None

    def _strip(self, line):
        groups = HeaderParser.REGEX_STRIP.match(line)
        if groups and groups.group(1):
            return groups.group(1)
        return ''

    def _parse_hint_name(self, line, regex, match_group, state_next):
        groups = regex.match(line)
        if not groups:
            raise RuntimeError('Cannot parse declaration')
        if not groups.group(2):
            self._current_hint_name = None
        else:
            self._current_hint_name = groups.group(match_group)
        self._state = state_next
        return groups

    def _on_document_finish(self):
        if self._state == PARSE_STATE_CLASS_LOOK_FOR_CLASS_NAME or self._state == PARSE_STATE_MODULE_LOOK_FOR_CLASS_NAME:
            raise RuntimeError('Class declaration expected, but found EOF')
        elif self._state == PARSE_STATE_CLASS_PARSE_METHOD_NAME or self._state == PARSE_STATE_CLASS_PARSE_GETTER_NAME or \
                self._state == PARSE_STATE_CLASS_PARSE_SETTER_NAME or self._state == PARSE_STATE_MODULE_PARSE_METHOD_NAME:
            raise RuntimeError('Member function declaration expected, but found EOF')
        self._state = PARSE_STATE_DEFAULT
        self._current_class = None
        self._current_module = None
        self._current_enum = None
        self._current_hint_name = None
        self._current_flag = None

    def _on_class_decl(self, line):
        groups = HeaderParser.REGEX_CLASS_HINT.match(line)
        if not groups:
            raise RuntimeError('Bad class declaration')
        self._state = PARSE_STATE_CLASS_LOOK_FOR_CLASS_NAME
        self._current_module = None
        self._current_enum = None
        self._current_flag = None

    def _on_parse_class_name(self, line):
        groups = HeaderParser.REGEX_CLASS_NAME_MATCH.match(line)
        if not groups:
            groups = HeaderParser.REGEX_CLASS_NAME_ALIAS_MATCH.match(line)
            if not groups:
                raise RuntimeError('Cannot parse class declaration')
        cls = ClassDecl(groups.group(2))
        if cls.get_name() in self._classes:
            raise RuntimeError('Class already defined (namespace is not supported yet)')
        self._classes[cls.get_name()] = cls
        self._current_class = cls
        self._state = PARSE_STATE_CLASS_LOOK_FOR_METHOD

    def _on_class_method_decl(self, line):
        self._parse_hint_name(line, HeaderParser.REGEX_CLASS_METHOD_HINT, 2, PARSE_STATE_CLASS_PARSE_METHOD_NAME)

    def _on_parse_class_method_name(self, line):
        native_name = _parse_native_method_name(line)
        if self._current_hint_name is None:
            self._current_hint_name = native_name
        self._current_class.add_method(self._current_hint_name, native_name)
        self._state = PARSE_STATE_CLASS_LOOK_FOR_METHOD

    def _on_class_getter_decl(self, line):
        self._parse_hint_name(line, HeaderParser.REGEX_CLASS_GETTER_HINT, 2, PARSE_STATE_CLASS_PARSE_GETTER_NAME)

    def _on_parse_class_getter_name(self, line):
        native_name = _parse_native_method_name(line)
        if self._current_hint_name is None:
            self._current_hint_name = native_name[3:] if native_name.startswith('Get') else native_name
        self._current_class.add_getter(self._current_hint_name, native_name)
        self._state = PARSE_STATE_CLASS_LOOK_FOR_METHOD

    def _on_class_setter_decl(self, line):
        self._parse_hint_name(line, HeaderParser.REGEX_CLASS_SETTER_HINT, 2, PARSE_STATE_CLASS_PARSE_SETTER_NAME)

    def _on_parse_class_setter_name(self, line):
        native_name = _parse_native_method_name(line)
        if self._current_hint_name is None:
            self._current_hint_name = native_name[3:] if native_name.startswith('Set') else native_name
        self._current_class.add_setter(self._current_hint_name, native_name)
        self._state = PARSE_STATE_CLASS_LOOK_FOR_METHOD

    def _on_module_decl(self, line):
        groups = self._parse_hint_name(line, HeaderParser.REGEX_MODULE_HINT, 2, PARSE_STATE_MODULE_LOOK_FOR_CLASS_NAME)
        self._current_class = None
        self._current_enum = None
        if groups.group(4):
            self._current_flag = groups.group(4)
        else:
            self._current_flag = None

    def _on_parse_module_name(self, line):
        groups = HeaderParser.REGEX_CLASS_NAME_MATCH.match(line)
        if not groups:
            raise RuntimeError('Cannot parse class declaration')
        module_name = groups.group(2) if self._current_hint_name is None else self._current_hint_name
        is_global = False
        if self._current_flag is not None:
            if self._current_flag == 'GLOBAL':
                is_global = True
            else:
                raise RuntimeError(f'Unknown flag {self._current_flag}')
        mod = ModuleDecl(module_name, groups.group(2), is_global)
        # 允许扩展同名 Module
        # if mod.get_name() in self._modules:
        #     raise RuntimeError('Module already defined')
        # self._modules[mod.get_name()] = mod
        self._modules.append(mod)
        self._current_module = mod
        self._state = PARSE_STATE_MODULE_LOOK_FOR_METHOD

    def _on_module_method_decl(self, line):
        self._parse_hint_name(line, HeaderParser.REGEX_CLASS_METHOD_HINT, 2, PARSE_STATE_MODULE_PARSE_METHOD_NAME)

    def _on_parse_module_method_name(self, line):
        native_name = _parse_native_method_name(line)
        if self._current_hint_name is None:
            self._current_hint_name = native_name
        self._current_module.add_method(self._current_hint_name, native_name)
        self._state = PARSE_STATE_MODULE_LOOK_FOR_METHOD

    def _on_module_enum_decl(self, line):
        self._parse_hint_name(line, HeaderParser.REGEX_MODULE_ENUM_HINT, 2, PARSE_STATE_MODULE_LOOK_FOR_ENUM_NAME)
        assert(self._current_module is not None)
        self._current_class = None
        self._current_flag = None

    def _on_parse_module_enum_name(self, line):
        groups = HeaderParser.REGEX_ENUM_NAME_MATCH.match(line)
        if not groups:
            raise RuntimeError('Cannot parse enum class declaration')
        enum_name = groups.group(2) if self._current_hint_name is None else self._current_hint_name
        self._current_enum = EnumDecl(enum_name, groups.group(2))
        self._current_module.add_enum(self._current_enum)
        self._state = PARSE_STATE_MODULE_LOOK_FOR_ENUM_FIELD

    def _on_enum_field(self, line):
        self._parse_hint_name(line, HeaderParser.REGEX_MODULE_ENUM_FIELD_HINT, 2, PARSE_STATE_MODULE_PARSE_ENUM_FIELD)

    def _on_parse_module_enum_field(self, line):
        groups = HeaderParser.REGEX_ENUM_FIELD_MATCH.match(line)
        if not groups:
            raise RuntimeError('Cannot parse enum field declaration')
        native_name = groups.group(1)
        if self._current_hint_name is None:
            self._current_hint_name = native_name
        self._current_enum.add_field(self._current_hint_name, native_name)
        self._state = PARSE_STATE_MODULE_LOOK_FOR_ENUM_FIELD

    def get_classes(self):
        return self._classes

    def get_modules(self):
        return self._modules

    def process(self, line):
        if line is None:
            self._on_document_finish()
            return

        line = self._strip(line)
        if line == '':
            return
        if self._state == PARSE_STATE_DEFAULT or self._state == PARSE_STATE_CLASS_LOOK_FOR_METHOD or \
                self._state == PARSE_STATE_MODULE_LOOK_FOR_METHOD or self._state == PARSE_STATE_MODULE_LOOK_FOR_ENUM_FIELD:
            if line.startswith('LSTG_CLASS'):
                self._on_class_decl(line)
            elif line.startswith('LSTG_MODULE'):
                self._on_module_decl(line)
            else:
                if self._state == PARSE_STATE_CLASS_LOOK_FOR_METHOD:
                    if line.startswith('LSTG_METHOD'):
                        self._on_class_method_decl(line)
                    elif line.startswith('LSTG_GETTER'):
                        self._on_class_getter_decl(line)
                    elif line.startswith('LSTG_SETTER'):
                        self._on_class_setter_decl(line)
                elif self._state == PARSE_STATE_MODULE_LOOK_FOR_METHOD or self._state == PARSE_STATE_MODULE_LOOK_FOR_ENUM_FIELD:
                    if line.startswith('LSTG_METHOD'):
                        self._on_module_method_decl(line)
                    elif line.startswith('LSTG_ENUM'):
                        self._on_module_enum_decl(line)
                    elif self._state == PARSE_STATE_MODULE_LOOK_FOR_ENUM_FIELD and line.startswith('LSTG_FIELD'):
                        self._on_enum_field(line)
        elif self._state == PARSE_STATE_CLASS_LOOK_FOR_CLASS_NAME:
            self._on_parse_class_name(line)
        elif self._state == PARSE_STATE_MODULE_LOOK_FOR_CLASS_NAME:
            self._on_parse_module_name(line)
        elif self._state == PARSE_STATE_CLASS_PARSE_METHOD_NAME:
            self._on_parse_class_method_name(line)
        elif self._state == PARSE_STATE_CLASS_PARSE_GETTER_NAME:
            self._on_parse_class_getter_name(line)
        elif self._state == PARSE_STATE_CLASS_PARSE_SETTER_NAME:
            self._on_parse_class_setter_name(line)
        elif self._state == PARSE_STATE_MODULE_PARSE_METHOD_NAME:
            self._on_parse_module_method_name(line)
        elif self._state == PARSE_STATE_MODULE_LOOK_FOR_ENUM_NAME:
            self._on_parse_module_enum_name(line)
        elif self._state == PARSE_STATE_MODULE_PARSE_ENUM_FIELD:
            self._on_parse_module_enum_field(line)


def main():
    cmd_parser = argparse.ArgumentParser(description='C++ to lua auto bridge tool for LuaSTGPlus engine.')
    cmd_parser.add_argument('--files', metavar='FILE', type=str, nargs='+', help='Scan header files')
    cmd_parser.add_argument('--output', type=str, required=True, help='Specify the output path of the auto generated bridge file')
    cmd_parser.add_argument('--namespace', type=str, required=False, nargs='+', help='Additional namespace for the output file')
    cmd_parser.add_argument('--name', type=str, required=True, help='The name of the auto bridge function method')
    cmd_parser.add_argument('--base', type=str, required=False, default='', help='Base directory for all the input file')
    cmd_args = cmd_parser.parse_args()

    # 解析所有输入的头文件
    header_parser = HeaderParser()
    with fileinput.input(files=cmd_args.files) as f:
        for line in f:
            header_parser.process(line)
        header_parser.process(None)

    # 计算包含头部
    includes = []
    base = ''
    if cmd_args.base != '':
        base = os.path.abspath(cmd_args.base)
    for path in cmd_args.files:
        abs_path = os.path.abspath(path)
        if base != '':
            includes.append(os.path.relpath(abs_path, base))
        else:
            includes.append(abs_path)

    # 输出类型绑定文件
    with open(cmd_args.output, 'w', encoding='utf-8') as f:
        f.write('/**\n')
        f.write(' * This file is auto generated by LSTGHeaderTool.\n')
        f.write(' * DO NOT MODIFY IT!\n')
        f.write(' */\n')
        # 头文件
        f.write(f'#include <lstg/Core/Subsystem/Script/LuaStack.hpp>\n')
        f.write(f'#include <lstg/Core/Subsystem/Script/LuaPush.hpp>\n')
        f.write(f'#include <lstg/Core/Subsystem/Script/LuaRead.hpp>\n')
        f.write(f'#include <lstg/Core/Subsystem/Script/LuaClassRegister.hpp>\n')
        f.write(f'#include <lstg/Core/Subsystem/Script/LuaModuleRegister.hpp>\n')
        for i in includes:
            f.write(f'#include <{i}>\n')
        f.write('\n')
        # 命名空间
        if cmd_args.namespace:
            for n in cmd_args.namespace:
                f.write(f'using namespace {n};\n')
        f.write('\n')
        f.write('using LuaStack = lstg::Subsystem::Script::LuaStack;\n')
        f.write('template <typename T>\n')
        f.write('using LuaClassRegister = lstg::Subsystem::Script::LuaClassRegister<T>;\n')
        f.write('using LuaModuleRegister = lstg::Subsystem::Script::LuaModuleRegister;\n')
        f.write('\n')
        # 类的注册函数
        f.write('namespace lstg::Subsystem::Script\n')
        f.write('{\n')
        for name in header_parser.get_classes():
            cls = header_parser.get_classes()[name]
            methods = cls.get_methods()
            getters = cls.get_getters()
            setters = cls.get_setters()
            f.write(f'void LuaRegister(LuaClassRegister<{name}>& register_)\n')
            f.write('{\n')
            if len(methods) == 0 and len(getters) == 0 and len(setters) == 0:
                f.write(f'    static_cast<void>(register_);\n')
            else:
                f.write(f'    register_\n')
                for method_name in methods:
                    method = methods[method_name]
                    f.write(f'        .Method("{method[0]}", &{name}::{method[1]})\n')
                for getter_name in getters:
                    getter = getters[getter_name]
                    if getter[0] in setters:
                        f.write(f'        .GetterSetter("{getter[0]}", &{name}::{getter[1]}, {name}::{setters[1]})\n')
                    else:
                        f.write(f'        .Getter("{getter[0]}", &{name}::{getter[1]})\n')
                for setter_name in setters:
                    setter = setters[setter_name]
                    f.write(f'        .Setter("{setter[0]}", &{name}::{setter[1]})\n')
                f.write(f'    ;\n')
            f.write('}\n')
        f.write('}\n')
        # Bridge 方法
        f.write(f'void {cmd_args.name}(LuaStack& stack_)\n')
        f.write('{\n')
        for module in header_parser.get_modules():
            # module = header_parser.get_modules()[name]
            name = module.get_name()
            native_class = module.get_native_class_name()
            is_global = module.is_global()
            methods = module.get_methods()
            enums = module.get_enums()
            f.write(f'    // Module register for {name}\n')
            f.write(f'    LuaModuleRegister(stack_, "{name}", {"true" if is_global else "false"})\n')
            for enum_name in enums:
                enum = enums[enum_name]  # type: EnumDecl
                native_enum = f'{native_class}::{enum.get_native_enum_name()}'
                enum_fields = enum.get_fields()
                f.write(f'        .Enum("{enum.get_name()}")\n')
                for field_name in enum_fields:
                    field = enum_fields[field_name]
                    f.write(f'            .Put("{field[0]}", {native_enum}::{field[1]})\n')
                f.write(f'            .End()\n')
            for method_name in methods:
                method = methods[method_name]
                f.write(f'        .Put("{method[0]}", &{native_class}::{method[1]})\n')
            f.write('    ;\n\n')
        # 强制注册所有类对象
        for name in header_parser.get_classes():
            cls = header_parser.get_classes()[name]
            f.write(f'    // Class register for {name}\n')
            f.write(f'    static_assert(lstg::Subsystem::Script::detail::HasLuaRegister<{name}>::value);\n')
            f.write(f'    if (luaL_newmetatable(stack_, lstg::Subsystem::Script::detail::GetUniqueTypeName<{name}>().Name.c_str()))\n')
            f.write(f'        lstg::Subsystem::Script::detail::NativeObjectRegister<{name}>::Register(stack_);\n')
            f.write(f'    stack_.Pop(1);\n\n')
        f.write('}\n')


if __name__ == "__main__":
    main()
