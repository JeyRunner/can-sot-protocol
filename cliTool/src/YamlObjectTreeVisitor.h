#pragma once

#include <string>
#include <optional>
using namespace std;



struct ValueNode {
    string name;
    string type;
    bool typeIsEnumType = false;
    string access;
    optional<string> defaultValue;

    static ValueNode fromYaml(string name, YAML::Node yamlNodeAttrs) {
        ValueNode valueNode;
        valueNode.name = name;
        valueNode.type = yamlNodeAttrs["type"].as<string>();
        valueNode.access = yamlNodeAttrs["access"].as<string>();
        if (yamlNodeAttrs["default"]) {
            valueNode.defaultValue = yamlNodeAttrs["default"].as<string>();
        }
        return valueNode;
    }

    optional<string> getTypeAsCppType() {
        if (type == "uint8") {
            return "TYPE_UINT8";
        }
        else if (type == "uint16") {
            return "TYPE_UINT16";
        }
        else if (type == "float32") {
            return "TYPE_F32";
        }
        else {
            return nullopt;
        }
    }


    optional<string> getAccessAsCppValueNodeAccStr(bool fromMasterPerspective) {
        string readWritable;
        if (fromMasterPerspective) {
            if (access == "r") {
                readWritable = "Readable";
            }
            if (access == "w") {
                readWritable = "Writable";
            }
        }
        else {
            if (access == "r") {
                readWritable = "Writable";
            }
            if (access == "w") {
                readWritable = "Readable";
            }
        }
        if (access == "rw") {
            readWritable = "ReadWritable";
        }
        if (readWritable.empty()) {
            return nullopt;
        }
        return readWritable;
    }
};


class YamlObjectTreeVisitor {
public:
    struct Context {
        string nodePath;
        unsigned int nodePathDepthLevel = 0;
        Context withPathAppended(string pathToAppend) {
            Context c = *this;
            c.nodePath += pathToAppend +".";
            c.nodePathDepthLevel = nodePathDepthLevel+1;
            return c;
        }
    };


    void acceptObjectTree(YAML::Node objectTreeYamlNode) {
        if (!objectTreeYamlNode.IsSequence()) {
            cout << "object_tree key is not a sequence" << endl;
            return;
        }
        acceptNodeWithChildren(objectTreeYamlNode, Context{.nodePath=""});
    }



    void acceptNodeWithChildren(YAML::Node node, Context context) {
        //cout << "-- " << context.nodePath << endl;
        for (const auto &el : node) {
            //cout << endl<< endl<< el << endl;

            if (el.IsSequence()) {
                auto nodeName = el.first.as<string>();
                visitNodeWithChildren(nodeName, el.begin()->second, context.withPathAppended(nodeName));
            }
            if (el.IsMap()) {
                string valNodeName;
                YAML::Node nodeAttrs;
                if (!el["name"] and el.size() == 1) {
                    // has sub-nodes
                    if (el.begin()->second.IsSequence()) {
                        auto nodeName = el.begin()->first.as<string>();
                        visitNodeWithChildren(nodeName, el.begin()->second, context.withPathAppended(nodeName));
                        continue;
                    }
                        // is value node
                    else if (el.begin()->second.IsMap()) {
                        valNodeName = el.begin()->first.as<string>();
                        nodeAttrs = el.begin()->second;
                    }
                    else {
                        cout << "Error: unsupported type in spec object_tree " << context.nodePath << endl;
                    }
                }
                    // is value node
                else if (el["name"]) {
                    valNodeName = el["name"].as<string>();
                    nodeAttrs = el.as<YAML::Node>();
                }
                else {
                    cout << "Error: wrong node object_tree " << context.nodePath << endl;
                }

                // is value Node
                acceptValueNode(valNodeName, nodeAttrs, context);
                //cout << "########## " << nodeName << endl;
            }
            else {
                cout << "Error: unsupported type in spec object_tree " << context.nodePath << endl;
            }
            //cout << el << endl << endl << endl;
        }
    }


private:
    void acceptValueNode(string nodeName, YAML::Node nodeAttrs, Context context) {
        auto valueNode = ValueNode::fromYaml(nodeName, nodeAttrs);
        visitValueNode(valueNode, context);
    }


public:
    virtual void visitNodeWithChildren(string nodeName, YAML::Node objectTreeYamlNode, Context context) {
        acceptNodeWithChildren(objectTreeYamlNode, context);
    }

    virtual void visitValueNode(ValueNode valueNode, Context context) {

    }
};