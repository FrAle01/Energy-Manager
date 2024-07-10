package com.unipi.dii.iot;

import java.net.InetAddress;
import java.util.List;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.simple.JSONObject;

import com.unipi.dii.iot.DatabaseManager.SensorIp;

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
    
            if (actuator != null && ipAddress != null) {

                InetAddress addr = exchange.getSourceAddress();
                
                System.out.println("Source address actautor: " + addr);
                System.out.println("actuator: " + actuator);

                // Remove the initial '/' from addr
                String addrWithoutSlash = addr.getHostAddress().substring(1);

                // Insert the sensor IP in the database
                try {

                    // Checking for the ip of actuator
                    if(db.elementRegistered(actuator, ipAddress)){
                        System.out.println("actuator IP already registered");
                        response = new Response(CoAP.ResponseCode.BAD_REQUEST);
                        exchange.respond(response);
                        return;
                    }

                    System.out.println("Inserting ACTUATOR IP in db " + ipAddress);
                    db.insertAddress(addrWithoutSlash, "actuator", actuator);
                    db.createActuatorTable(actuator, addrWithoutSlash);

                    System.out.println("actuator IP REGISTERED!");

                    // Create response JSON object
                    JSONObject responseJson = new JSONObject();
                    // taking ips from database


                    List<SensorIp> list = db.getSensorAddress();
                    for (SensorIp sensor : list) {

                        System.out.println("sensor: " + sensor.name + " ip: " + sensor.address);
                        responseJson.put(sensor.name, sensor.address);

                    }
                    System.out.println("indirizzi ip sensori per l'attuatore " + responseJson.toString());

                    response = new Response(CoAP.ResponseCode.CREATED);
                    response.setPayload(responseJson.toJSONString());
                    exchange.respond(response);
                    

                    // strarting observer for energyflow
                    System.out.println("Starting observer client for energy flow");  
                    final CoAPObserver observerClient = new CoAPObserver(ipAddress, "energyflow");
                    Thread observertThread=new Thread(observerClient);
                    observertThread.start();


                } catch (Exception e) {
                    System.err.println("Error inserting sensor IP in the database: " + e.getMessage());
                    response = new Response(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
                    exchange.respond(response);
                }
            } else {
                System.err.println("Missing required key 'a' or IP address");
                response = new Response(CoAP.ResponseCode.BAD_REQUEST);
                exchange.respond(response);
            }
    }

}
