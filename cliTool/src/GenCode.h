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
#include "filesystem"

#include "GenCodeYamlStructs.h"

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

    /// the key is the remote call name, value is the id
    map<string, string> remoteCallsIds;

  private:
    GenObjectTreeCode genObjectTreeCode;
    set<string> definedEnums;

    list<RemoteCall> remoteCallsCallable; // other device can call func on this device
    list<RemoteCall> remoteCallsCaller;   // this device is the caller


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
          hbox({text(L" outputHeaderDir:   "), text(cliArgs.outputHeaderFilesDir) | bold}) | color(Color::Green),
          hbox({text(L" protocol name:          "),text(protocolName) | bold}) | color(Color::Green),
          hbox({text(L" generate for:           "),text(genTarget) | bold}) | color(Color::Green),
      }));
      genObjectTreeCode.forMaster = cliArgs.forMaster;

      filesystem::create_directories(cliArgs.outputHeaderFilesDir);

      //ryml::Tree specYaml = ryml::parse_in_place(s.c_str());
      YAML::Node spec = YAML::LoadFile(cliArgs.inputDefFileName);


      // read value node ids table
      for (auto el : spec["object_tree_node_ids"]) {
          valueNodeIds[el.second.as<string>()] = el.first.as<string>();
      }
      cout << ">> valueNodeIds: " << endl;
      cout << YAML::Node(valueNodeIds) << endl;

      // read remote calls ids table
      for (auto el : spec["remote_calls_ids"]) {
        remoteCallsIds[el.second.as<string>()] = el.first.as<string>();
      }
      cout << ">> remoteCallsIds: " << endl;
      cout << YAML::Node(remoteCallsIds) << endl << endl;

      // sort remote calls
      remoteCallSortCallerCallable(spec["remote_calls"]);


      // gen enum defs and save enum names
      string enumDefsCode = genEnumDefs(spec["global_defs"]["enums"]);

      // gen code for object tree
      spec["object_tree"].push_back(spec["object_tree_predefined"]);
      genObjectTreeCode.acceptObjectTree(spec["object_tree"]);

      // gen code for remote calls
      string remoteCallsDataStructsDefCode = genRemoteCallsDataStructs();
      string remoteCallsDefCode = genRemoteCallsDef();

      // insert all
      string protocolHeader = insertIntoTemplate(genCodeTemplateHeaderContent, {
          {"GENERATED_CMD", argsString},
          {"GENERATION_TARGET", genTarget},
          {"PROTOCOL_CLASS_NAME", firstCharToUpper(protocolName)},
          {"ENUM_DEFS", enumDefsCode},
          {"OBJECT_TREE", genObjectTreeCode.code},
          {"REMOTE_CALLS", remoteCallsDefCode},
          {"NODE_ID_TABLE_SIZE", to_string(valueNodeIds.size())},
          {"NODE_ID_TABLE_CONTENT", genOtNodeIDsTableContent()},
          {"RCCALLER_TABLE_SIZE", to_string(remoteCallsCaller.size())},
          {"RCCALLABLE_TABLE_SIZE", to_string(remoteCallsCallable.size())},
          {"REMOTE_CALLS_CALLER_TABLE_CONTENT", genRemoteCallsTables(true)},
          {"REMOTE_CALLS_CALlABLE_TABLE_CONTENT", genRemoteCallsTables(false)},
          {"CONSTRUCTOR_SETUP_ALL_NODE_VALUES", genConstructorSetup()},
          {"NODES_TO_SEND_ON_INIT_TABLE_SIZE", "0"}, // @todo
          {"NODES_TO_SEND_ON_INIT_TABLE_CONTENT", ""}, // @todo
      });

      if (genObjectTreeCode.error) {
          printEl(hbox({">> "_T | bold, text("there were errors, will not save generated code")}) | color(Color::Red));
          //return;
      }

      //printEl(hbox({">> "_T | bold, text("generated code (" + to_string(protocolHeader.size()) + " characters)")}) | color(Color::Green));
      //cout << header << endl;

      // main protocol header
      string outputDefFileName = cliArgs.outputHeaderFilesDir + "/" + protocolName + ".hpp";
      ofstream outputDefFile(outputDefFileName);
      outputDefFile << protocolHeader;
      outputDefFile.flush();
      outputDefFile.close();
      printEl(hbox({">> "_T | bold, text("saved header file "+ outputDefFileName)}) | color(Color::Green));


      // remote call data structs header
      string structsHeader = insertIntoTemplate(genCodeTemplate_StructsHeader, {
              {"GENERATED_CMD", argsString},
              {"GENERATION_TARGET", genTarget},
              {"PROTOCOL_CLASS_NAME", firstCharToUpper(protocolName)},
              {"REMOTE_CALLS_DATA_STRUCTS", remoteCallsDataStructsDefCode},
      });
      string outputStructsHeaderFileName = cliArgs.outputHeaderFilesDir + "/" + protocolName + "_Structs.hpp";
      ofstream outputStructsFile(outputStructsHeaderFileName);
      outputStructsFile << structsHeader;
      outputStructsFile.flush();
      outputStructsFile.close();
      printEl(hbox({">> "_T | bold, text("saved header file "+ outputStructsHeaderFileName)}) | color(Color::Green));
    }


    void remoteCallSortCallerCallable(YAML::Node remoteCalls) {
      if (remoteCalls.size() == 0) {
        printEl(hbox({">> "_T | bold, text("No remote calls defined.")}));
        return;
      }
      if (!remoteCalls.IsSequence()) {
        errorAndExit("remote_calls has to be a list, but its not.", "remote_calls");
        return;
      }
      for (const auto &callYaml: remoteCalls) {
        RemoteCall call = RemoteCall::readFromYaml(callYaml);
        if (call.direction == "master_to_client") {
          if (cliArgs.forMaster) {
            remoteCallsCaller.push_back(call);
          }
          else {
            remoteCallsCallable.push_back(call);
          }
        }
        else {
          errorAndExit("for remote_calls currently only master_to_client is supported for 'direction'.", "remote_calls");
          return;
        }
      }
    }


    string genRemoteCallsDef() {
      auto getEnumOrVoidType = [&](string retErrorType) -> string {
        if (retErrorType.empty()) {
          return "VOID_ENUM";
        }
        else {
          return getTypeAsEnum(retErrorType);
        }
      };

      string t;
      for (auto call : remoteCallsCaller) {
        string id = getIdForRemoteCalls(call.name);
        t += "\t\tRemoteCallCaller<"+ id +", "+ call.typeArgsData +", "+ call.typeReturnData +", "+ getEnumOrVoidType(call.return_value_error) +", COMC> "+ call.name +";\n";
      }
      for (auto call : remoteCallsCallable) {
        string id = getIdForRemoteCalls(call.name);
        t += "\t\tRemoteCallCallable<"+ id +", "+ call.typeArgsData +", "+ call.typeReturnData +", "+ getEnumOrVoidType(call.return_value_error) +", COMC> "+ call.name +";\n";
      }
      return t;
    }


    string genRemoteCallsDataStructs() {
      auto genCallDataStruct = [&](string name, string genCodeTemplate, string dataConvertFunc, list<RemoteCall::Arg> &members) {
        string typeName = firstCharToUpper(name);
        string membersCode;
        string convertDataCode;
        string convertDataCodeDataSizeAcc;
        string argsConstructorCode = name + "(";
        string argsConstructorCodeInitMembers;
        int i = 0;
        for (auto el : members) {
          bool isLast = i >= members.size()-1;
          bool isFist = i == 0;
          string cppType = getTypeAsCppTypeResolveEnums(el.type);
          string cppTypeShort = getTypeAsCppTypeResolveEnums(el.type, false);
          if (cppType.empty()) {
            errorAndExit("invalid type used in remote_calls args or return values", "remote_calls");
          }
          membersCode += "\t" + cppType + " " + el.name + ";\n";

          convertDataCode += "\t\t" + dataConvertFunc + cppTypeShort +
                  "(data["+ (convertDataCodeDataSizeAcc.empty() ? "0" : convertDataCodeDataSizeAcc) +"], "
                  + el.name +");\n";
          convertDataCodeDataSizeAcc += string("") + (isFist ? "" : " + ") + "sizeof(" + cppType + ")";

          argsConstructorCode += cppType + " " + el.name + (isLast ? "" : ",");
          argsConstructorCodeInitMembers += el.name + "(" + el.name + ")"+ (isLast ? "" : ", ");
          i++;
        }
        argsConstructorCode += ")";
        argsConstructorCodeInitMembers += " {}";

        return insertIntoTemplate(std::move(genCodeTemplate), {
                {"NAME_TYPE", name},
                {"MEMBERS", membersCode},
                {"CONSTRUCTOR_WITH_ARGS", argsConstructorCode + ": " + argsConstructorCodeInitMembers},
                {"REQUIRED_SIZE", convertDataCodeDataSizeAcc},
                {"DATA_CONVERSION", convertDataCode},
        });
      };

      string t;
      for (auto &call : remoteCallsCaller) {
        if (!call.args.empty()) {
          call.typeArgsData = firstCharToUpper(call.name) + "ArgDataCaller";
          t += genCallDataStruct(call.typeArgsData, genCodeTemplate_RemoteCallDataWritable, "writeToData", call.args);
        }
        else {
          call.typeArgsData = "VoidRemoteCallDataWritable";
        }
        // return data
        if (!call.return_values_ok.empty()) {
          call.typeReturnData = firstCharToUpper(call.name) + "ReturnDataCaller";
          t += genCallDataStruct(call.typeReturnData, genCodeTemplate_RemoteCallDataReadable, "readFromData", call.args);
        }
        else {
          call.typeReturnData = "VoidRemoteCallDataReadable";
        }
        t += "\n";
      }

      for (auto &call : remoteCallsCallable) {
        if (!call.args.empty()) {
          call.typeArgsData = firstCharToUpper(call.name) + "ArgDataCallable";
          t += genCallDataStruct(call.typeArgsData, genCodeTemplate_RemoteCallDataReadable, "readFromData", call.args);
        }
        else {
          call.typeArgsData = "VoidRemoteCallDataReadable";
        }
        // return data
        if (!call.return_values_ok.empty()) {
          call.typeReturnData = firstCharToUpper(call.name) + "ReturnDataCallable";
          t += genCallDataStruct(call.typeReturnData, genCodeTemplate_RemoteCallDataWritable, "writeToData", call.args);
        }
        else {
          call.typeReturnData = "VoidRemoteCallDataWritable";
        }
        t += "\n";
      }
      return t;
    }


    /**
     * Gen entries for rcCallerTable or rcCallableTable.
     * @param isCaller is false gen for callable
     * @return
     */
    string genRemoteCallsTables(bool isCaller) {
      string t;
      for (int i=0; i<remoteCallsIds.size(); i++) {
        auto [name, id] = *std::find_if(remoteCallsIds.begin(), remoteCallsIds.end(), [&](auto &el){return el.second == to_string(i);});
        if (isCaller && std::find_if(remoteCallsCaller.begin(), remoteCallsCaller.end(), [&](auto &el){return el.name == name;}) != remoteCallsCaller.end()) {
          t += "\t\t" + insertIntoTemplate(genCodeTemplate_RemoteCallsTableEntry_Caller_noComma, {{"PATH", "remoteCalls." + name}}) + ",\n";
        }
        else if ((!isCaller) && std::find_if(remoteCallsCallable.begin(), remoteCallsCallable.end(), [&](auto &el){return el.name == name;}) != remoteCallsCallable.end()) {
          t += "\t\t" + insertIntoTemplate(genCodeTemplate_RemoteCallsTableEntry_Callable_noComma, {{"PATH", "remoteCalls." + name}}) + ",\n";
        }
        else {
          continue;
        }
      }
      return t;
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
            t += "\t\t" + insertIntoTemplate(genCodeTemplate_IdTableEntry_noComma, {{"NODE_PATH", "objectTree." + path}}) + ",\n";
        }
        return t;
    }

    string genConstructorSetup() {
        string t;
        for (auto [path, id] : valueNodeIds) {
            t += "\t\t" + insertIntoTemplate(genCodeTemplate_ConstructorSetupEntry, {{"NODE_PATH", "objectTree." + path}}) + ";\n";
        }
        for (auto [name, id] : remoteCallsIds) {
          t += "\t\t" + insertIntoTemplate(genCodeTemplate_ConstructorSetupEntry, {{"NODE_PATH", "remoteCalls." + name}}) + ";\n";
        }
        return t;
    }


    string getTypeAsCppTypeResolveEnums(string typeStr, bool typePrefix=true) {
      auto type = getTypeAsCppType(typeStr, typePrefix);
      string resultType;
      if (!type) {
        resultType = getTypeAsEnum(typeStr);
      }
      else {
        resultType = type.value();
      }
      return resultType;
    }

    string getTypeAsEnum(string typeStr) {
      string resultType;
      // if its enum
      if (checkIfEnumIsDefined(typeStr)) {
        resultType = typeStr;
      }
      else {
        cout << ">> Error: type '"<< typeStr <<"' is unknown, allowed are: uint8, uint16, float32 or a defined enum" << endl;
        return "";
      }
      return resultType;
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

    string getIdForRemoteCalls(string callName) {
      if (!remoteCallsIds.contains(callName)) {
        cout << endl;
        printEl(hbox({">> "_T | bold, text("can't find remote call node '" + callName + "' in remote_calls_ids ")}) | color(Color::Red));
        cout << "  remote_calls_ids table: " << YAML::Node(remoteCallsIds) << endl;
        throw runtime_error("can't find remote call id");
      }
      return remoteCallsIds[callName];
    }

    bool checkIfEnumIsDefined(string enumName) {
        return definedEnums.contains(enumName);
    }


    string getProtocolNameFromFilename(string path) {
      filesystem::path filePath(path);
      string filename = filePath.filename();

      auto loc = filename.find(".");
      if (loc == std::string::npos) {
          cout << "filename does not contain a '.', will just use the full filename as protocol name: " << path << endl;
          return filename;
      }
      auto s = filename.substr(0, loc);
      std::replace(s.begin(), s.end(), '-', '_');
      return firstCharToUpper(s);
    }



};



