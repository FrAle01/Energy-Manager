package com.unipi.dii.iot;

import java.time.LocalDateTime;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.simple.JSONObject;

public class CoAPResourceTimestamp extends CoapResource{

    public CoAPResourceTimestamp(String name){
        super(name);
    }

    @Override
    public void handleGET(CoapExchange exchange){
        exchange.respond("hello world");
    }

    @Override
    public void handlePOST(CoapExchange exchange){
        
        System.out.println("Timestamp Request received");

        String payload = exchange.getRequestText();
        String ipAddress = exchange.getSourceAddress().getHostAddress();
        
        System.out.println("Payload received: " + payload + " \nlength: " + payload.length());
        System.out.println("IP address: " + ipAddress + "\n");
    
        Response response;          
    
        if (payload.equals("time")) {
                
                JSONObject responseJson = new JSONObject();
                
                LocalDateTime now = LocalDateTime.now();
                int month = now.getMonthValue();
                int day = now.getDayOfMonth();
                int hour = now.getHour();
            
                System.out.println("month: " + month + ", day: " + day + ", hour: " + hour);
                responseJson.put("month", month);
                responseJson.put("day", day);
                responseJson.put("hour", hour);

                System.out.println("timestamp values for actuator:  " + responseJson.toString());
                response = new Response(CoAP.ResponseCode.CREATED);
                response.setPayload(responseJson.toJSONString());
                exchange.respond(response);
                
                
        
        } else {
            System.err.println("Bad payload for timestamp request");
            response = new Response(CoAP.ResponseCode.BAD_REQUEST);
            exchange.respond(response);
        }
        

    }


}
