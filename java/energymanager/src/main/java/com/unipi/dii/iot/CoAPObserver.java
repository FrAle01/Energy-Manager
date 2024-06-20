package com.unipi.dii.iot;


import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;


public class CoAPObserver implements Runnable {

    private CoapClient client;
    private  CoapObserveRelation relation;
    private String sensor;
    private String ipv6;
    

    public CoAPObserver(String address,String sensor) {
       
        // CoAP client constructor
        String uri = "coap://[" + address + "]:5683/monitoring";
        client = new CoapClient(uri);
        this.ipv6=address;
        this.sensor=sensor;
    

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

                try{
                   JSONParser parser = new JSONParser();
                   if(sensor.equals("temmperature")){
                    json = (JSONObject) parser.parse(content);
                    Long timeid=(Long) json.get("id");
                    JSONArray ssArray =(JSONArray) json.get("ss");
                    db.insertSensorTHERMOMETER("thermometer", ipv6, ssArray);
                   }
                   else if(sensor.equals("lpgSensor")) {
                    json = (JSONObject) parser.parse(content);
                    if (json.containsKey("ss")) {
                        Long timeid = (Long) json.get("id");
                        JSONArray ssArray = (JSONArray) json.get("ss");
                        db.insertSensorLPG("lpgSensor", ipv6,ssArray, timeid);
                    } else {
                        System.out.println("Il JSON non contiene 'ss'");
                    }
                }

                } catch (Exception e) {
                    e.printStackTrace();
                }
              
            }

            @Override
            public void onError() {
                System.err.println("Failed to receive notification");
                
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

