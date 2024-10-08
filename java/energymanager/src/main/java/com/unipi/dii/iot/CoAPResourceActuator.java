package com.unipi.dii.iot;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;

public class CoAPResourceActuator extends CoapResource{

    DatabaseManager db = new DatabaseManager();

    public CoAPResourceActuator(String name){
        super(name);
        setObservable(true);
    }

    @Override
    public void handleGET(CoapExchange exchange){
        exchange.respond("hello world");
    }

    @Override
    public void handlePOST(CoapExchange exchange){
        
        if(!db.allSensorsOnline()){

            System.out.println("NOT ALL SENSORS REGISTERED YET!");
            Response response = new Response(CoAP.ResponseCode.BAD_REQUEST);
            exchange.respond(response);
            return;
        }        
        
        System.out.println("POST ACTUATOR received");

        String payload = exchange.getRequestText();
        String ipAddress = exchange.getSourceAddress().getHostAddress();
        
        System.out.println("Payload received: " + payload + " \nlength: " + payload.length());
        System.out.println("IP address: " + ipAddress + "\n");
    
        Response response;
    
        String actuator = payload;
    
            if (actuator != null && ipAddress != null && !db.elementRegistered(payload, ipAddress)) {

                
                System.out.println("Source address actautor: " + ipAddress);
                System.out.println("actuator: " + actuator);

                // Remove the initial '/' from addr

                // Insert the sensor IP in the database
                try {

                    // Checking for the ip of actuator

                    System.out.println("Inserting ACTUATOR IP in db " + ipAddress);
                    db.insertAddress(ipAddress, "actuator", actuator);
                    db.createActuatorTable(actuator, ipAddress);

                    System.out.println("actuator IP REGISTERED!");

                    response = new Response(CoAP.ResponseCode.CREATED);
                    response.setPayload("Registration completed");
                    exchange.respond(response);


                } catch (Exception e) {
                    System.err.println("Error inserting sensor IP in the database: " + e.getMessage());
                    response = new Response(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
                    exchange.respond(response);
                }
            } else {
                System.err.println("Missing IP address or element already registered");
                response = new Response(CoAP.ResponseCode.BAD_REQUEST);
                exchange.respond(response);
            }
    }

}
