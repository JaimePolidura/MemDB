package es.memdb.messages.response;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class ResponseDeserializerTest  {
    private ResponseDeserializer responseDeserializer;

    @Before
    public void setup() {
        this.responseDeserializer = new ResponseDeserializer();
    }

    @Test
    public void shouldDeserializeError() {
        byte[] toDeserialize = new byte[]{
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                0x02
        };

        Response response = this.responseDeserializer.deserialize(toDeserialize);

        Assert.assertFalse(response.isSuccessful());
        Assert.assertEquals(1, response.getTimestamp());
        Assert.assertEquals(1, response.getRequestNumber());
        Assert.assertEquals(1, response.getErrorCode());
        Assert.assertEquals("", response.getResponse());
    }


    @Test
    public void shouldDeserializeSuccess() {
        byte[] toDeserialize = new byte[]{
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                0x01,
                0x02,
                0x4F, 0x4B
        };

        Response response = this.responseDeserializer.deserialize(toDeserialize);

        Assert.assertTrue(response.isSuccessful());
        Assert.assertEquals(1, response.getTimestamp());
        Assert.assertEquals(1, response.getRequestNumber());
        Assert.assertEquals("OK", response.getResponse());
    }
}
