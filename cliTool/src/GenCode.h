#pragma once

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include "CliArgs.h"
#include "Util.h"
#include "YamlObjectTreeVisitor.h"
#include "GenCodeTemplate.h"

using namespace std;
using namespace ftxui;

class GenCode;


class GenObjectTreeCode: public YamlObjectTreeVisitor {
public:
    string code;
    function<string (string valueNodePath)> getIdForValueNode;
    function<bool (string enumName)> checkIfEnumIsDefined;
    bool forMaster;
    bool error = false;

    void visitNodeWithChildren(string nodeName, YAML::Node objectTreeYamlNode, Context context) override {
        code += string(context.nodePathDepthLevel+1, '\t') + "struct " + firstCharToUpper(nodeName) + ": Node {\n";
        YamlObjectTreeVisitor::visitNodeWithChildren(nodeName, objectTreeYamlNode, context);
        code += string(context.nodePathDepthLevel+1, '\t') + "} " + nodeName + "; \n";
    }

    void visitValueNode(ValueNode valueNode, Context context) override {
        string fullPath = context.nodePath+valueNode.name;
        cout << "## " << context.nodePath << "@" << valueNode.name << " -> " << getIdForValueNode(fullPath) << endl;
        auto readWritable = valueNode.getAccessAsCppValueNodeAccStr(forMaster);
        if (!readWritable) {
            cout << ">> Error: node '"<< fullPath <<"' has unknown access value, allowed are: r, w, rw" << endl;
            error = true;
        }
        auto type = valueNode.getTypeAsCppType();
        string typeStr = "";
        string readWritableStr = readWritable.value();
        if (!type) {
            // if its enum
            if (checkIfEnumIsDefined(valueNode.type)) {
                typeStr = valueNode.type;
                readWritableStr += "Enum";
            }
            else {
                cout << ">> Error: node '"<< fullPath <<"' has unknown type value, allowed are: uint8, uint16, float32 or a defined enum" << endl;
                error = true;
            }
        }
        else {
            typeStr = type.value();
        }
        code += string(context.nodePathDepthLevel+2, '\t') + insertIntoTemplate(genCodeTemplate_ValueNode, {
            {"READ_WRITABLE", readWritableStr},
            {"TYPE", typeStr},
            {"ID", getIdForValueNode(fullPath)},
            {"NAME", valueNode.name},
        });
    }
};


class GenCode{
  public:
    CliArgs::GenProtocolCodeCommand cliArgs;
    /// the key is the value node path, value is the id
    map<string, string> valueNodeIds;

  private:
    GenObjectTreeCode genObjectTreeCode;
    set<string> definedEnums;

  public:
    GenCode(CliArgs::GenProtocolCodeCommand cliArgs)
    : cliArgs(cliArgs) {
        // (bind(&GenCode::getIdForValueNode, *this, std::placeholders::_1));
        genObjectTreeCode.getIdForValueNode = [this](auto && PH1) { return getIdForValueNode(std::forward<decltype(PH1)>(PH1)); };
        genObjectTreeCode.checkIfEnumIsDefined = [this](auto && PH1) { return checkIfEnumIsDefined(std::forward<decltype(PH1)>(PH1)); };
    }


    void genCode(vector<string> cliCmd) {
      string argsString;
      for (auto a : cliCmd) {argsString += a + " ";}
      string genTarget = cliArgs.forMaster ? "master" : "client";

      //printTest();
      printEl(hbox({">> "_T | bold, "will generate def file from spec file ... "_T}));
      string protocolName = getProtocolNameFromFilename(cliArgs.inputDefFileName);
      printEl_asWindow("Parameters", vbox({
          hbox({text(L" inputDefFileName:       "),text(cliArgs.inputDefFileName) | bold}) | color(Color::Green),
          hbox({text(L" outputHeaderFileName:   "),text(cliArgs.outputHeaderFileName) | bold}) | color(Color::Green),
          hbox({text(L" protocol name:          "),text(protocolName) | bold}) | color(Color::Green),
          hbox({text(L" generate for:           "),text(genTarget) | bold}) | color(Color::Green),
      }));
      genObjectTreeCode.forMaster = cliArgs.forMaster;

      //ryml::Tree specYaml = ryml::parse_in_place(s.c_str());
      // read value node ids table
      YAML::Node spec = YAML::LoadFile(cliArgs.inputDefFileName);
      for (auto el : spec["object_tree_node_ids"]) {
          valueNodeIds[el.second.as<string>()] = el.first.as<string>();
      }
      cout << YAML::Node(valueNodeIds) << endl;

      // gen enum defs and save enum names
      string enumDefsCode = genEnumDefs(spec["global_defs"]["enums"]);

      // gen code for object tree
      spec["object_tree"].push_back(spec["object_tree_predefined"]);
      genObjectTreeCode.acceptObjectTree(spec["object_tree"]);

      // insert all
      string header = insertIntoTemplate(genCodeTemplateHeaderContent, {
          {"GENERATED_CMD", argsString},
          {"GENERATION_TARGET", genTarget},
          {"PROTOCOL_CLASS_NAME", firstCharToUpper(protocolName)},
          {"ENUM_DEFS", enumDefsCode},
          {"OBJECT_TREE", genObjectTreeCode.code},
          {"NODE_ID_TABLE_SIZE", to_string(valueNodeIds.size())},
          {"NODE_ID_TABLE_CONTENT", genOtNodeIDsTableContent()},
          {"CONSTRUCTOR_SETUP_ALL_NODE_VALUES", genConstructorSetup()},
          {"NODES_TO_SEND_ON_INIT_TABLE_SIZE", "0"}, // @todo
          {"NODES_TO_SEND_ON_INIT_TABLE_CONTENT", ""}, // @todo
      });

      if (genObjectTreeCode.error) {
          printEl(hbox({">> "_T | bold, text("there were errors, will not save generated code")}) | color(Color::Red));
          //return;
      }

      printEl(hbox({">> "_T | bold, text("generated code ("+ to_string(header.size())+" characters)")}) | color(Color::Green));
      //cout << header << endl;

      ofstream outputDefFile(cliArgs.outputHeaderFileName);
      outputDefFile << header;
      outputDefFile.flush();
      outputDefFile.close();
    }



    string genEnumDefs(YAML::Node enumDefYaml) {
        if (!enumDefYaml.IsSequence()) {
            printEl(hbox({">> Error: "_T | bold, text("global_defs.enums has to be a list, but its not.")}) | color(Color::Red));
            throw runtime_error("gen enum defs");
        }
        string t;
        for (const auto &enumDef: enumDefYaml) {
            if (!enumDef.IsMap() || enumDef.size() != 1) {
                printEl(hbox({">> Error: "_T | bold, text("global_defs.enums elements have to be objects with a single key which is the enum name.")}) | color(Color::Red));
                throw runtime_error("gen enum defs");
            }
            if (!enumDef.begin()->second.IsMap()) {
                printEl(hbox({">> Error: "_T | bold, text("global_defs.enums should be like: {ENUM_NAME: {VAL1: 1, VAL2: 2}}.")}) | color(Color::Red));
                throw runtime_error("gen enum defs");
            }
            auto name = enumDef.begin()->first.as<string>();
            if (definedEnums.contains(name)) {
                throw runtime_error("gen enum defs: enum "+ name+ " was defined multiple times.");
            }
            definedEnums.emplace(name);
            t += "enum class " + name + " {\n";
            for (auto [enumKey, enumValue]: enumDef.begin()->second.as<map<string, int>>()) {
                t += "\t" + enumKey + " = " + to_string(enumValue) + ",\n";
            }
            t += "};\n";
        }
        return t;
    };

    string genOtNodeIDsTableContent() {
        string t;
        for (int i=0; i<valueNodeIds.size(); i++) {
            auto [path, id] = *std::find_if(valueNodeIds.begin(), valueNodeIds.end(), [&](auto &el){return el.second == to_string(i);});
            t += "\t\t" + insertIntoTemplate(genCodeTemplate_IdTableEntry_noComma, {{"NODE_PATH", path}}) + ",\n";
        }
        return t;
    }

    string genConstructorSetup() {
        string t;
        for (auto [path, id] : valueNodeIds) {
            t += "\t\t" + insertIntoTemplate(genCodeTemplate_ConstructorSetupEntry, {{"NODE_PATH", "objectTree." + path}}) + ",\n";
        }
        return t;
    }




    string getIdForValueNode(string valueNodePath) {
        if (!valueNodeIds.contains(valueNodePath)) {
            cout << endl;
            printEl(hbox({">> "_T | bold, text("can't find value node '" + valueNodePath + "' in object_tree_node_ids ")}) | color(Color::Red));
            cout << "  node id table: " << YAML::Node(valueNodeIds) << endl;
            throw runtime_error("can't find value node id");
        }
        return valueNodeIds[valueNodePath];
    }

    bool checkIfEnumIsDefined(string enumName) {
        return definedEnums.contains(enumName);
    }


    string getProtocolNameFromFilename(string filename) {
        auto loc = filename.find(".");
        if (loc == std::string::npos) {
            cout << "filename does not contain a '.', will just use the full filename as protocol name: " << filename << endl;
            return filename;
        }
        auto s = filename.substr(0, loc);
        return firstCharToUpper(s);
    }
};



