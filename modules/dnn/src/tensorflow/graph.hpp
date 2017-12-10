#ifndef __OPENCV_DNN_TENSORFLOW_GRAPH_PROTO_HPP__
#define __OPENCV_DNN_TENSORFLOW_GRAPH_PROTO_HPP__
const std::string kTensorflowGraphProto = std::string("") +
"file {\n"
"  name: \"tensor_shape.proto\"\n"
"  package: \"tensorflow\"\n"
"  message_type {\n"
"    name: \"TensorShapeProto\"\n"
"    field {\n"
"      name: \"dim\"\n"
"      number: 2\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.TensorShapeProto.Dim\"\n"
"      json_name: \"dim\"\n"
"    }\n"
"    field {\n"
"      name: \"unknown_rank\"\n"
"      number: 3\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BOOL\n"
"      json_name: \"unknownRank\"\n"
"    }\n"
"    nested_type {\n"
"      name: \"Dim\"\n"
"      field {\n"
"        name: \"size\"\n"
"        number: 1\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_INT64\n"
"        json_name: \"size\"\n"
"      }\n"
"      field {\n"
"        name: \"name\"\n"
"        number: 2\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"name\"\n"
"      }\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"TensorShapeProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}\n"
"file {\n"
"  name: \"types.proto\"\n"
"  package: \"tensorflow\"\n"
"  enum_type {\n"
"    name: \"DataType\"\n"
"    value {\n"
"      name: \"DT_INVALID\"\n"
"      number: 0\n"
"    }\n"
"    value {\n"
"      name: \"DT_FLOAT\"\n"
"      number: 1\n"
"    }\n"
"    value {\n"
"      name: \"DT_DOUBLE\"\n"
"      number: 2\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT32\"\n"
"      number: 3\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT8\"\n"
"      number: 4\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT16\"\n"
"      number: 5\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT8\"\n"
"      number: 6\n"
"    }\n"
"    value {\n"
"      name: \"DT_STRING\"\n"
"      number: 7\n"
"    }\n"
"    value {\n"
"      name: \"DT_COMPLEX64\"\n"
"      number: 8\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT64\"\n"
"      number: 9\n"
"    }\n"
"    value {\n"
"      name: \"DT_BOOL\"\n"
"      number: 10\n"
"    }\n"
"    value {\n"
"      name: \"DT_QINT8\"\n"
"      number: 11\n"
"    }\n"
"    value {\n"
"      name: \"DT_QUINT8\"\n"
"      number: 12\n"
"    }\n"
"    value {\n"
"      name: \"DT_QINT32\"\n"
"      number: 13\n"
"    }\n"
"    value {\n"
"      name: \"DT_BFLOAT16\"\n"
"      number: 14\n"
"    }\n"
"    value {\n"
"      name: \"DT_QINT16\"\n"
"      number: 15\n"
"    }\n"
"    value {\n"
"      name: \"DT_QUINT16\"\n"
"      number: 16\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT16\"\n"
"      number: 17\n"
"    }\n"
"    value {\n"
"      name: \"DT_COMPLEX128\"\n"
"      number: 18\n"
"    }\n"
"    value {\n"
"      name: \"DT_HALF\"\n"
"      number: 19\n"
"    }\n"
"    value {\n"
"      name: \"DT_RESOURCE\"\n"
"      number: 20\n"
"    }\n"
"    value {\n"
"      name: \"DT_VARIANT\"\n"
"      number: 21\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT32\"\n"
"      number: 22\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT64\"\n"
"      number: 23\n"
"    }\n"
"    value {\n"
"      name: \"DT_FLOAT_REF\"\n"
"      number: 101\n"
"    }\n"
"    value {\n"
"      name: \"DT_DOUBLE_REF\"\n"
"      number: 102\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT32_REF\"\n"
"      number: 103\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT8_REF\"\n"
"      number: 104\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT16_REF\"\n"
"      number: 105\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT8_REF\"\n"
"      number: 106\n"
"    }\n"
"    value {\n"
"      name: \"DT_STRING_REF\"\n"
"      number: 107\n"
"    }\n"
"    value {\n"
"      name: \"DT_COMPLEX64_REF\"\n"
"      number: 108\n"
"    }\n"
"    value {\n"
"      name: \"DT_INT64_REF\"\n"
"      number: 109\n"
"    }\n"
"    value {\n"
"      name: \"DT_BOOL_REF\"\n"
"      number: 110\n"
"    }\n"
"    value {\n"
"      name: \"DT_QINT8_REF\"\n"
"      number: 111\n"
"    }\n"
"    value {\n"
"      name: \"DT_QUINT8_REF\"\n"
"      number: 112\n"
"    }\n"
"    value {\n"
"      name: \"DT_QINT32_REF\"\n"
"      number: 113\n"
"    }\n"
"    value {\n"
"      name: \"DT_BFLOAT16_REF\"\n"
"      number: 114\n"
"    }\n"
"    value {\n"
"      name: \"DT_QINT16_REF\"\n"
"      number: 115\n"
"    }\n"
"    value {\n"
"      name: \"DT_QUINT16_REF\"\n"
"      number: 116\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT16_REF\"\n"
"      number: 117\n"
"    }\n"
"    value {\n"
"      name: \"DT_COMPLEX128_REF\"\n"
"      number: 118\n"
"    }\n"
"    value {\n"
"      name: \"DT_HALF_REF\"\n"
"      number: 119\n"
"    }\n"
"    value {\n"
"      name: \"DT_RESOURCE_REF\"\n"
"      number: 120\n"
"    }\n"
"    value {\n"
"      name: \"DT_VARIANT_REF\"\n"
"      number: 121\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT32_REF\"\n"
"      number: 122\n"
"    }\n"
"    value {\n"
"      name: \"DT_UINT64_REF\"\n"
"      number: 123\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"TypesProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}\n"
"file {\n"
"  name: \"tensor.proto\"\n"
"  package: \"tensorflow\"\n"
"  dependency: \"tensor_shape.proto\"\n"
"  dependency: \"types.proto\"\n"
"  message_type {\n"
"    name: \"TensorProto\"\n"
"    field {\n"
"      name: \"dtype\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_ENUM\n"
"      type_name: \".tensorflow.DataType\"\n"
"      json_name: \"dtype\"\n"
"    }\n"
"    field {\n"
"      name: \"tensor_shape\"\n"
"      number: 2\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.TensorShapeProto\"\n"
"      json_name: \"tensorShape\"\n"
"    }\n"
"    field {\n"
"      name: \"version_number\"\n"
"      number: 3\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_INT32\n"
"      json_name: \"versionNumber\"\n"
"    }\n"
"    field {\n"
"      name: \"tensor_content\"\n"
"      number: 4\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BYTES\n"
"      json_name: \"tensorContent\"\n"
"    }\n"
"    field {\n"
"      name: \"half_val\"\n"
"      number: 13\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_INT32\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"halfVal\"\n"
"    }\n"
"    field {\n"
"      name: \"float_val\"\n"
"      number: 5\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_FLOAT\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"floatVal\"\n"
"    }\n"
"    field {\n"
"      name: \"double_val\"\n"
"      number: 6\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_DOUBLE\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"doubleVal\"\n"
"    }\n"
"    field {\n"
"      name: \"int_val\"\n"
"      number: 7\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_INT32\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"intVal\"\n"
"    }\n"
"    field {\n"
"      name: \"string_val\"\n"
"      number: 8\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_BYTES\n"
"      json_name: \"stringVal\"\n"
"    }\n"
"    field {\n"
"      name: \"scomplex_val\"\n"
"      number: 9\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_FLOAT\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"scomplexVal\"\n"
"    }\n"
"    field {\n"
"      name: \"int64_val\"\n"
"      number: 10\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_INT64\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"int64Val\"\n"
"    }\n"
"    field {\n"
"      name: \"bool_val\"\n"
"      number: 11\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_BOOL\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"boolVal\"\n"
"    }\n"
"    field {\n"
"      name: \"dcomplex_val\"\n"
"      number: 12\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_DOUBLE\n"
"      options {\n"
"        packed: true\n"
"      }\n"
"      json_name: \"dcomplexVal\"\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"TensorProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}\n"
"file {\n"
"  name: \"attr_value.proto\"\n"
"  package: \"tensorflow\"\n"
"  dependency: \"tensor.proto\"\n"
"  dependency: \"tensor_shape.proto\"\n"
"  dependency: \"types.proto\"\n"
"  message_type {\n"
"    name: \"AttrValue\"\n"
"    field {\n"
"      name: \"s\"\n"
"      number: 2\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BYTES\n"
"      oneof_index: 0\n"
"      json_name: \"s\"\n"
"    }\n"
"    field {\n"
"      name: \"i\"\n"
"      number: 3\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_INT64\n"
"      oneof_index: 0\n"
"      json_name: \"i\"\n"
"    }\n"
"    field {\n"
"      name: \"f\"\n"
"      number: 4\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_FLOAT\n"
"      oneof_index: 0\n"
"      json_name: \"f\"\n"
"    }\n"
"    field {\n"
"      name: \"b\"\n"
"      number: 5\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BOOL\n"
"      oneof_index: 0\n"
"      json_name: \"b\"\n"
"    }\n"
"    field {\n"
"      name: \"type\"\n"
"      number: 6\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_ENUM\n"
"      type_name: \".tensorflow.DataType\"\n"
"      oneof_index: 0\n"
"      json_name: \"type\"\n"
"    }\n"
"    field {\n"
"      name: \"shape\"\n"
"      number: 7\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.TensorShapeProto\"\n"
"      oneof_index: 0\n"
"      json_name: \"shape\"\n"
"    }\n"
"    field {\n"
"      name: \"tensor\"\n"
"      number: 8\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.TensorProto\"\n"
"      oneof_index: 0\n"
"      json_name: \"tensor\"\n"
"    }\n"
"    field {\n"
"      name: \"list\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.AttrValue.ListValue\"\n"
"      oneof_index: 0\n"
"      json_name: \"list\"\n"
"    }\n"
"    field {\n"
"      name: \"func\"\n"
"      number: 10\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.NameAttrList\"\n"
"      oneof_index: 0\n"
"      json_name: \"func\"\n"
"    }\n"
"    field {\n"
"      name: \"placeholder\"\n"
"      number: 9\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      oneof_index: 0\n"
"      json_name: \"placeholder\"\n"
"    }\n"
"    nested_type {\n"
"      name: \"ListValue\"\n"
"      field {\n"
"        name: \"s\"\n"
"        number: 2\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_BYTES\n"
"        json_name: \"s\"\n"
"      }\n"
"      field {\n"
"        name: \"i\"\n"
"        number: 3\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_INT64\n"
"        options {\n"
"          packed: true\n"
"        }\n"
"        json_name: \"i\"\n"
"      }\n"
"      field {\n"
"        name: \"f\"\n"
"        number: 4\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_FLOAT\n"
"        options {\n"
"          packed: true\n"
"        }\n"
"        json_name: \"f\"\n"
"      }\n"
"      field {\n"
"        name: \"b\"\n"
"        number: 5\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_BOOL\n"
"        options {\n"
"          packed: true\n"
"        }\n"
"        json_name: \"b\"\n"
"      }\n"
"      field {\n"
"        name: \"type\"\n"
"        number: 6\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_ENUM\n"
"        type_name: \".tensorflow.DataType\"\n"
"        options {\n"
"          packed: true\n"
"        }\n"
"        json_name: \"type\"\n"
"      }\n"
"      field {\n"
"        name: \"shape\"\n"
"        number: 7\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_MESSAGE\n"
"        type_name: \".tensorflow.TensorShapeProto\"\n"
"        json_name: \"shape\"\n"
"      }\n"
"      field {\n"
"        name: \"tensor\"\n"
"        number: 8\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_MESSAGE\n"
"        type_name: \".tensorflow.TensorProto\"\n"
"        json_name: \"tensor\"\n"
"      }\n"
"    }\n"
"    oneof_decl {\n"
"      name: \"value\"\n"
"    }\n"
"  }\n"
"  message_type {\n"
"    name: \"NameAttrList\"\n"
"    field {\n"
"      name: \"name\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"name\"\n"
"    }\n"
"    field {\n"
"      name: \"attr\"\n"
"      number: 2\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.NameAttrList.AttrEntry\"\n"
"      json_name: \"attr\"\n"
"    }\n"
"    nested_type {\n"
"      name: \"AttrEntry\"\n"
"      field {\n"
"        name: \"key\"\n"
"        number: 1\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"key\"\n"
"      }\n"
"      field {\n"
"        name: \"value\"\n"
"        number: 2\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_MESSAGE\n"
"        type_name: \".tensorflow.AttrValue\"\n"
"        json_name: \"value\"\n"
"      }\n"
"      options {\n"
"        map_entry: true\n"
"      }\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"AttrValueProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}\n"
"file {\n"
"  name: \"op_def.proto\"\n"
"  package: \"tensorflow\"\n"
"  dependency: \"attr_value.proto\"\n"
"  dependency: \"types.proto\"\n"
"  message_type {\n"
"    name: \"OpDef\"\n"
"    field {\n"
"      name: \"name\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"name\"\n"
"    }\n"
"    field {\n"
"      name: \"input_arg\"\n"
"      number: 2\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.OpDef.ArgDef\"\n"
"      json_name: \"inputArg\"\n"
"    }\n"
"    field {\n"
"      name: \"output_arg\"\n"
"      number: 3\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.OpDef.ArgDef\"\n"
"      json_name: \"outputArg\"\n"
"    }\n"
"    field {\n"
"      name: \"attr\"\n"
"      number: 4\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.OpDef.AttrDef\"\n"
"      json_name: \"attr\"\n"
"    }\n"
"    field {\n"
"      name: \"deprecation\"\n"
"      number: 8\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.OpDeprecation\"\n"
"      json_name: \"deprecation\"\n"
"    }\n"
"    field {\n"
"      name: \"summary\"\n"
"      number: 5\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"summary\"\n"
"    }\n"
"    field {\n"
"      name: \"description\"\n"
"      number: 6\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"description\"\n"
"    }\n"
"    field {\n"
"      name: \"is_commutative\"\n"
"      number: 18\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BOOL\n"
"      json_name: \"isCommutative\"\n"
"    }\n"
"    field {\n"
"      name: \"is_aggregate\"\n"
"      number: 16\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BOOL\n"
"      json_name: \"isAggregate\"\n"
"    }\n"
"    field {\n"
"      name: \"is_stateful\"\n"
"      number: 17\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BOOL\n"
"      json_name: \"isStateful\"\n"
"    }\n"
"    field {\n"
"      name: \"allows_uninitialized_input\"\n"
"      number: 19\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_BOOL\n"
"      json_name: \"allowsUninitializedInput\"\n"
"    }\n"
"    nested_type {\n"
"      name: \"ArgDef\"\n"
"      field {\n"
"        name: \"name\"\n"
"        number: 1\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"name\"\n"
"      }\n"
"      field {\n"
"        name: \"description\"\n"
"        number: 2\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"description\"\n"
"      }\n"
"      field {\n"
"        name: \"type\"\n"
"        number: 3\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_ENUM\n"
"        type_name: \".tensorflow.DataType\"\n"
"        json_name: \"type\"\n"
"      }\n"
"      field {\n"
"        name: \"type_attr\"\n"
"        number: 4\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"typeAttr\"\n"
"      }\n"
"      field {\n"
"        name: \"number_attr\"\n"
"        number: 5\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"numberAttr\"\n"
"      }\n"
"      field {\n"
"        name: \"type_list_attr\"\n"
"        number: 6\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"typeListAttr\"\n"
"      }\n"
"      field {\n"
"        name: \"is_ref\"\n"
"        number: 16\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_BOOL\n"
"        json_name: \"isRef\"\n"
"      }\n"
"    }\n"
"    nested_type {\n"
"      name: \"AttrDef\"\n"
"      field {\n"
"        name: \"name\"\n"
"        number: 1\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"name\"\n"
"      }\n"
"      field {\n"
"        name: \"type\"\n"
"        number: 2\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"type\"\n"
"      }\n"
"      field {\n"
"        name: \"default_value\"\n"
"        number: 3\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_MESSAGE\n"
"        type_name: \".tensorflow.AttrValue\"\n"
"        json_name: \"defaultValue\"\n"
"      }\n"
"      field {\n"
"        name: \"description\"\n"
"        number: 4\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"description\"\n"
"      }\n"
"      field {\n"
"        name: \"has_minimum\"\n"
"        number: 5\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_BOOL\n"
"        json_name: \"hasMinimum\"\n"
"      }\n"
"      field {\n"
"        name: \"minimum\"\n"
"        number: 6\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_INT64\n"
"        json_name: \"minimum\"\n"
"      }\n"
"      field {\n"
"        name: \"allowed_values\"\n"
"        number: 7\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_MESSAGE\n"
"        type_name: \".tensorflow.AttrValue\"\n"
"        json_name: \"allowedValues\"\n"
"      }\n"
"    }\n"
"  }\n"
"  message_type {\n"
"    name: \"OpDeprecation\"\n"
"    field {\n"
"      name: \"version\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_INT32\n"
"      json_name: \"version\"\n"
"    }\n"
"    field {\n"
"      name: \"explanation\"\n"
"      number: 2\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"explanation\"\n"
"    }\n"
"  }\n"
"  message_type {\n"
"    name: \"OpList\"\n"
"    field {\n"
"      name: \"op\"\n"
"      number: 1\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.OpDef\"\n"
"      json_name: \"op\"\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"OpDefProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}\n"
"file {\n"
"  name: \"function.proto\"\n"
"  package: \"tensorflow\"\n"
"  dependency: \"attr_value.proto\"\n"
"  dependency: \"op_def.proto\"\n"
"  message_type {\n"
"    name: \"FunctionDefLibrary\"\n"
"    field {\n"
"      name: \"function\"\n"
"      number: 1\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.FunctionDef\"\n"
"      json_name: \"function\"\n"
"    }\n"
"    field {\n"
"      name: \"gradient\"\n"
"      number: 2\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.GradientDef\"\n"
"      json_name: \"gradient\"\n"
"    }\n"
"  }\n"
"  message_type {\n"
"    name: \"FunctionDef\"\n"
"    field {\n"
"      name: \"signature\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.OpDef\"\n"
"      json_name: \"signature\"\n"
"    }\n"
"    field {\n"
"      name: \"node\"\n"
"      number: 2\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.FunctionDef.Node\"\n"
"      json_name: \"node\"\n"
"    }\n"
"    nested_type {\n"
"      name: \"Node\"\n"
"      field {\n"
"        name: \"ret\"\n"
"        number: 1\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_STRING\n"
"        json_name: \"ret\"\n"
"      }\n"
"      field {\n"
"        name: \"op\"\n"
"        number: 2\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"op\"\n"
"      }\n"
"      field {\n"
"        name: \"arg\"\n"
"        number: 3\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_STRING\n"
"        json_name: \"arg\"\n"
"      }\n"
"      field {\n"
"        name: \"dep\"\n"
"        number: 4\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_STRING\n"
"        json_name: \"dep\"\n"
"      }\n"
"      field {\n"
"        name: \"attr\"\n"
"        number: 5\n"
"        label: LABEL_REPEATED\n"
"        type: TYPE_MESSAGE\n"
"        type_name: \".tensorflow.FunctionDef.Node.AttrEntry\"\n"
"        json_name: \"attr\"\n"
"      }\n"
"      nested_type {\n"
"        name: \"AttrEntry\"\n"
"        field {\n"
"          name: \"key\"\n"
"          number: 1\n"
"          label: LABEL_OPTIONAL\n"
"          type: TYPE_STRING\n"
"          json_name: \"key\"\n"
"        }\n"
"        field {\n"
"          name: \"value\"\n"
"          number: 2\n"
"          label: LABEL_OPTIONAL\n"
"          type: TYPE_MESSAGE\n"
"          type_name: \".tensorflow.AttrValue\"\n"
"          json_name: \"value\"\n"
"        }\n"
"        options {\n"
"          map_entry: true\n"
"        }\n"
"      }\n"
"    }\n"
"  }\n"
"  message_type {\n"
"    name: \"GradientDef\"\n"
"    field {\n"
"      name: \"function_name\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"functionName\"\n"
"    }\n"
"    field {\n"
"      name: \"gradient_func\"\n"
"      number: 2\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"gradientFunc\"\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"FunctionProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}\n"
"file {\n"
"  name: \"versions.proto\"\n"
"  package: \"tensorflow\"\n"
"  message_type {\n"
"    name: \"VersionDef\"\n"
"    field {\n"
"      name: \"producer\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_INT32\n"
"      json_name: \"producer\"\n"
"    }\n"
"    field {\n"
"      name: \"min_consumer\"\n"
"      number: 2\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_INT32\n"
"      json_name: \"minConsumer\"\n"
"    }\n"
"    field {\n"
"      name: \"bad_consumers\"\n"
"      number: 3\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_INT32\n"
"      json_name: \"badConsumers\"\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"VersionsProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}\n"
"file {\n"
"  name: \"graph.proto\"\n"
"  package: \"tensorflow\"\n"
"  dependency: \"attr_value.proto\"\n"
"  dependency: \"function.proto\"\n"
"  dependency: \"versions.proto\"\n"
"  message_type {\n"
"    name: \"GraphDef\"\n"
"    field {\n"
"      name: \"node\"\n"
"      number: 1\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.NodeDef\"\n"
"      json_name: \"node\"\n"
"    }\n"
"    field {\n"
"      name: \"versions\"\n"
"      number: 4\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.VersionDef\"\n"
"      json_name: \"versions\"\n"
"    }\n"
"    field {\n"
"      name: \"version\"\n"
"      number: 3\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_INT32\n"
"      options {\n"
"        deprecated: true\n"
"      }\n"
"      json_name: \"version\"\n"
"    }\n"
"    field {\n"
"      name: \"library\"\n"
"      number: 2\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.FunctionDefLibrary\"\n"
"      json_name: \"library\"\n"
"    }\n"
"  }\n"
"  message_type {\n"
"    name: \"NodeDef\"\n"
"    field {\n"
"      name: \"name\"\n"
"      number: 1\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"name\"\n"
"    }\n"
"    field {\n"
"      name: \"op\"\n"
"      number: 2\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"op\"\n"
"    }\n"
"    field {\n"
"      name: \"input\"\n"
"      number: 3\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_STRING\n"
"      json_name: \"input\"\n"
"    }\n"
"    field {\n"
"      name: \"device\"\n"
"      number: 4\n"
"      label: LABEL_OPTIONAL\n"
"      type: TYPE_STRING\n"
"      json_name: \"device\"\n"
"    }\n"
"    field {\n"
"      name: \"attr\"\n"
"      number: 5\n"
"      label: LABEL_REPEATED\n"
"      type: TYPE_MESSAGE\n"
"      type_name: \".tensorflow.NodeDef.AttrEntry\"\n"
"      json_name: \"attr\"\n"
"    }\n"
"    nested_type {\n"
"      name: \"AttrEntry\"\n"
"      field {\n"
"        name: \"key\"\n"
"        number: 1\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_STRING\n"
"        json_name: \"key\"\n"
"      }\n"
"      field {\n"
"        name: \"value\"\n"
"        number: 2\n"
"        label: LABEL_OPTIONAL\n"
"        type: TYPE_MESSAGE\n"
"        type_name: \".tensorflow.AttrValue\"\n"
"        json_name: \"value\"\n"
"      }\n"
"      options {\n"
"        map_entry: true\n"
"      }\n"
"    }\n"
"  }\n"
"  options {\n"
"    java_package: \"org.tensorflow.framework\"\n"
"    java_outer_classname: \"GraphProtos\"\n"
"    java_multiple_files: true\n"
"    cc_enable_arenas: true\n"
"  }\n"
"  syntax: \"proto3\"\n"
"}";
#endif  // __OPENCV_DNN_TENSORFLOW_GRAPH_PROTO_HPP__
