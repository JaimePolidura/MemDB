#include "gtest/gtest.h"
#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include <string>
#include "../../Assertions.h"

Request createRequestNodeId(std::shared_ptr<std::vector<SimpleString<defaultMemDbLength_t>>> args, std::string authKey,
                            uint16_t nodeId, uint8_t opNumber, uint64_t timestamp, uint64_t reqNumber, bool flag1Auth = false,
                            bool flag2Auth = false, bool flag1Op = false, bool flag2Op = false);

/**
 * Authkey: 123
 * nodeId: 1
 * opNumber: 5
 * timestamp: 511,
 * reqNumber: 2
 */
TEST(RequestSerializer, WithArgsAndNodeId) {
    RequestSerializer requestSerializer{};
    auto args = std::make_shared<std::vector<SimpleString<defaultMemDbLength_t>>>();
    args->push_back(SimpleString<defaultMemDbLength_t>::fromChar(0x01));
    args->push_back(SimpleString<defaultMemDbLength_t>::fromChar(0x01));
    Request request = createRequestNodeId(args, "123", 1, 5, 0x1FF, 2); //00000001 11111111

    auto actual = requestSerializer.serialize(request, true);
    auto expected = std::vector<uint8_t>{
        0x00, 0x00, 0x00, 0x1D,
        0x00, 0x00, 0x00, 0x02, //Req number
        0x0C, 0x31, 0x32, 0x33, //Auth
        0x14, //Op number
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, //Timestamp
        0x00, 0x01, //Node id,
        0x00, 0x00, 0x00, 0x01, 0x01, //Length arg 1 -> arg
        0x00, 0x00, 0x00, 0x01, 0x01, //Length arg 2 -> arg
    };

    Assertions::assertFirstItemsVectorsEqual(expected, actual);
}

/**
 * Authkey: 123
 * opNumber: 5
 * timestamp: 511,
 * reqNumber: 2
 * Flag 2 auth : on
 * Flag 1 op : on
 */
TEST(RequestSerializer, WithoutArgsAndFlags) {
    RequestSerializer requestSerializer{};
    auto args = std::make_shared<std::vector<SimpleString<defaultMemDbLength_t>>>();
    Request request = createRequestNodeId(args, "123", 0, 5, 0x1FF, 2, false, true, true, false);
    request.authenticationType = AuthenticationType::USER;

    auto actual = requestSerializer.serialize(request, false);
    auto expected = std::vector<uint8_t>{
            0x00, 0x00, 0x00, 0x11,
            0x00, 0x00, 0x00, 0x02, //Req number
            0x0D, 0x31, 0x32, 0x33, //Auth
            0x16, //Op number
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, //Timestamp
    };

    Assertions::assertFirstItemsVectorsEqual(expected, actual);
}

Request createRequestNodeId(std::shared_ptr<std::vector<SimpleString<defaultMemDbLength_t>>> args, std::string authKey,
                            uint16_t nodeId, uint8_t opNumber, uint64_t timestamp, uint64_t reqNumber, bool flag1Auth,
                            bool flag2Auth, bool flag1Op, bool flag2Op) {
    AuthenticationBody authenticationBody{};
    authenticationBody.authKey = authKey;
    authenticationBody.flag1 = flag1Auth;
    authenticationBody.flag2 = flag2Auth;

    OperationBody operationBody;
    operationBody.args = args;
    operationBody.nodeId = nodeId;
    operationBody.timestamp = timestamp;
    operationBody.operatorNumber = opNumber;
    operationBody.flag1 = flag1Op;
    operationBody.flag2 = flag2Op;


    Request request{};
    request.authentication = authenticationBody;
    request.requestNumber = reqNumber;
    request.operation = operationBody;

    return request;
}