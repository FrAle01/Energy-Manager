package com.unipi.dii.iot;


import java.time.LocalDate;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;


public class CoAPObserver implements Runnable {

    private CoapClient client;
    private  CoapObserveRelation relation;
    private String resource;
    private String ipv6;
    

    public CoAPObserver(String address,String resource) {
       
        // CoAP client constructor
        String uri = "coap://[" + address + "]:5683/"+ resource;
        client = new CoapClient(uri);
        this.ipv6=address;
        this.resource=resource;
    

    }

    public void startObserving() {

        // Start resource observation
        relation = client.observe(new CoapHandler() {
            @Override
            public void onLoad(CoapResponse response){
                
                DatabaseManager db = new DatabaseManager();
                String content = response.getResponseText();
                System.out.println("Notification: " + content);
                JSONObject json= null;

                if(resource.equals("energyflow")){
                    try{
                        JSONParser parser = new JSONParser();
                        json = (JSONObject) parser.parse(content);
    
                        String str_home =(String) json.get("h");
                        String str_battery =(String) json.get("b");
                        String str_grid =(String) json.get("g");
                        String hour = (String) json.get("ts");

                        String date = LocalDate.now().toString();
                        String ts = date + "_" + hour;


                        Double to_home = Double.parseDouble(str_home);
                        Double to_battery = Double.parseDouble(str_battery);
                        Double to_grid = Double.parseDouble(str_grid);


                        Double produced = to_home + to_battery + to_grid;
                        Boolean added = db.insertFlowValues("inverter", ipv6, produced, to_home, to_battery, to_grid, ts);
    
                        if(added){
                            System.out.println("Energy flow new values (production: "+ produced +", home: "+ to_home +", battery: "+ to_battery +", grid: "+ to_grid+") inserted");
                        }else{
                            System.err.println("Failed to insert new flow");
                        }
    
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }else{

                    try{
                        JSONParser parser = new JSONParser();
                        json = (JSONObject) parser.parse(content);
    
                        String sensing =(String) json.get("sensor");

                        if(sensing.equals("tm")){
                            sensing = "temperature";
                        }else if(sensing.equals("ir")){
                            sensing = "irradiance";
                        }else if(sensing.equals("cp")){
                            sensing = "capacity";
                        }else if(sensing.equals("cn")){
                            sensing = "consumption";
                        }

                        String str_value =(String) json.get("value");
                        String ts =(String) json.get("ts");

                        Double value = Double.parseDouble(str_value);
                        
                        Boolean added = db.insertSensorValue(sensing, ipv6, value, ts);
    
                        if(added){
                            System.out.println("Sensor "+sensing+" new value ("+value+") inserted");
                        }else{
                            System.err.println("Failed to insert new value");
                        }
    
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
              
            }

            @Override
            public void onError() {
                DatabaseManager db = new DatabaseManager();
                
                System.err.println("Failed to receive notification");
                db.deleteAddress(ipv6, resource);
                
            }
        });
    }

    public void stopObserving() {
        if (relation != null) {
            relation.proactiveCancel();
        }
        if (client != null) {
            client.shutdown();
        }
    }
    @Override
    public void run() {
        startObserving();
    }
}

