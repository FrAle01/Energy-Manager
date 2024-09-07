package com.unipi.dii.iot;

import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.coap.CoAP.ResponseCode;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;

public class MyServer extends CoapServer{

    public static void main(String args[]) {
        
        DatabaseManager dbManager = new DatabaseManager(); 
        
        dbManager.deleteDB();
        dbManager.createDB();
        dbManager.createAddressTable();

        MyServer server = new MyServer();
        server.add(new CoAPResourceSensor("registrationSensor"));
        server.add(new CoAPResourceActuator("registrationActuator"));
        server.add(new CoAPResourceAddress("addressRequest"));
        server.add(new CoAPResourceTimestamp("timeRequest"));

        server.start();
        System.out.print("The server is Running!\n");
    }

    public void handleGET(CoapExchange exchange) {

        Response response = new Response(ResponseCode.CONTENT);
        if (exchange.getRequestOptions().getAccept() == MediaTypeRegistry.APPLICATION_XML) {
            response.getOptions().setContentFormat(MediaTypeRegistry.APPLICATION_XML);
            response.setPayload("<value>10</value>");
        } else if (exchange.getRequestOptions().getAccept() == MediaTypeRegistry.APPLICATION_JSON) {
            response.getOptions().setContentFormat(MediaTypeRegistry.APPLICATION_JSON);
            // ...
        }
        exchange.respond(response);
    }

}
