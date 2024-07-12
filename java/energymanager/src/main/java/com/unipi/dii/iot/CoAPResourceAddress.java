package com.unipi.dii.iot;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

public class CoAPResourceAddress extends CoapResource{

    DatabaseManager db = new DatabaseManager();
    private int sensors_addr_sent;

    public CoAPResourceAddress(String name){
        super(name);
        sensors_addr_sent = 0;
    }

    @Override
    public void handleGET(CoapExchange exchange){
        exchange.respond("hello world");
    }

    @Override
    public void handlePOST(CoapExchange exchange){
        
        System.out.println("Sensor Address Request received");

        String payload = exchange.getRequestText();
        String ipAddress = exchange.getSourceAddress().getHostAddress();
        
        System.out.println("Payload received: " + payload + " \nlength: " + payload.length());
        System.out.println("IP address: " + ipAddress + "\n");
    
        Response response;
    
        JSONObject json= null;
        try {
            JSONParser parser = new JSONParser();
            json = (JSONObject) parser.parse(payload);

            String actuator =(String) json.get("a");
            String sensor =(String) json.get("s");
            System.out.println("Parsed element: "+actuator+" requesting address for sensor: "+sensor);

    
            if (actuator != null && ipAddress != null && db.elementRegistered(actuator, ipAddress)) {
                    // actuator must be registered to receive sensors addresses
                    
                    // Checking for the ip of actuator
                    // Create response JSON object
                    JSONObject responseJson = new JSONObject();
                    // taking ips from database
                    String address = db.getAddress("sensor", sensor);
                    
                    if(address == null){
                        System.out.println("Failed to retrieve "+sensor+ " address");
                        response = new Response(CoAP.ResponseCode.BAD_REQUEST);
                        exchange.respond(response);

                    }else{
                        System.out.println("sensor: " + sensor + " addr: " + address);
                        responseJson.put("sensor", sensor);
                        responseJson.put("addr", address);
    
                        System.out.println("indirizzi ip sensori per l'attuatore " + responseJson.toString());
                        response = new Response(CoAP.ResponseCode.CREATED);
                        response.setPayload(responseJson.toJSONString());
                        exchange.respond(response);

                    }
                    
                    sensors_addr_sent++;
                    if (sensors_addr_sent >= 4) {    // sended address for less than 4 sensors
                        // strarting observer for energyflow
                        System.out.println("Starting observer client for energy flow");  
                        final CoAPObserver observerClient = new CoAPObserver(ipAddress, "energyflow");
                        Thread observertThread=new Thread(observerClient);
                        observertThread.start();
                    }
                    
            
            } else {
                System.err.println("Missing IP address or element NOT registered");
                response = new Response(CoAP.ResponseCode.BAD_REQUEST);
                exchange.respond(response);
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.err.println("Error finding sensor ip in the database: " + e.getMessage());
            response = new Response(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
            exchange.respond(response);
        }

    }


}
