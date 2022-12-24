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
                .authentication(AuthenticationRequest.builder()
                        .authKey("123")
                        .build())
                .operationRequest(OperationRequest.builder()
                        .operator(Operator.SET)
                        .args(List.of(
                                "name", "jaime"
                        ))
                        .build())
                .build();

        byte[] response = this.requestSerializer.serialize(request);
        byte[] expected = new byte[]{ 0x0C, 0x31, 0x32, 0x33, 0x04, 0x04, 0x6E, 0x61, 0x6D, 0x65, 0x05, 0x6A, 0x61, 0x69, 0x6D, 0x65};

        Assert.assertEquals(16, response.length);
        Assert.assertArrayEquals(response, expected);
    }
}
