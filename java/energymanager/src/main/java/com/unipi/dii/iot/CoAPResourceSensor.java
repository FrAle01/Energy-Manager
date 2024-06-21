package com.unipi.dii.iot;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;


public class CoAPResourceSensor extends CoapResource{

    DatabaseManager db = new DatabaseManager();

    public CoAPResourceSensor(String name){
        super(name);
        setObservable(true);
    }

    @Override
    public void handleGET(CoapExchange exchange){
        exchange.respond("hello world");
    }

    @Override
    public void handlePOST(CoapExchange exchange){

        System.out.println("POST sensor received");
        String sensor = exchange.getRequestText();
        System.out.println("Payload received: " + sensor + " \nlength: " + sensor.length());
        String ipAddress=exchange.getSourceAddress().getHostAddress();    

        try {

            Response response=null;

            if (sensor != null && ipAddress != null && !db.elementRegistered(sensor, ipAddress)) {

                // checking if sensor is already registered

                System.out.println("Inserting sensor IP in the "+ sensor);
                db.insertAddress(ipAddress, "sensor", sensor.toLowerCase());
                //insert sensor in the database
                db.createSensorTable(sensor.toLowerCase(), ipAddress);
                response = new Response(CoAP.ResponseCode.CREATED);
                exchange.respond(response);
                System.out.println("success\n");
                // Start the CoAP observer client
                
                System.out.println("Starting observer client for " );  
                final CoAPObserver observerClient = new CoAPObserver(ipAddress,sensor);
                Thread observertThread=new Thread(observerClient);
                observertThread.start();
                
               
        } else {
            System.err.println("Failed sensor registration");
            response = new Response(CoAP.ResponseCode.BAD_REQUEST);
        }
        } catch (Exception e) {
            System.err.println("Error inserting sensor IP in the database: " + e.getMessage());
            exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR, "generic error");
        }

    }
}
