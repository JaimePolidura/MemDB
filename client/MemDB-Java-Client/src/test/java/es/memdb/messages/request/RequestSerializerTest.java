package es.memdb.messages.request;

import es.memdb.Operator;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.util.List;

public class RequestSerializerTest {
    private RequestSerializer requestSerializer;

    @Before
    public void setup() {
        this.requestSerializer = new RequestSerializer();
    }

    @Test
    public void shouldSerialize() {
        Request request = Request.builder()
                .requestNumber(1)
                .authentication(AuthenticationRequest.builder()
                        .authClientKey("123")
                        .build())
                .operationRequest(OperationRequest.builder()
                        .operator(Operator.SET)
                        .args(List.of(
                                "name", "jaime"
                        ))
                        .build())
                .build();

        byte[] response = this.requestSerializer.serialize(request, 1);

        byte[] expected = new byte[]{
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Request number
                0x0C, 0x31, 0x32, 0x33, //Authentication
                0x04, //Operator desc
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Timestamp
                0x04, 0x6E, 0x61, 0x6D, 0x65, //Arg 1º
                0x05, 0x6A, 0x61, 0x69, 0x6D, 0x65 //Arg 2º
        };

        Assert.assertEquals(32, response.length);
        Assert.assertArrayEquals(response, expected);
    }
}
