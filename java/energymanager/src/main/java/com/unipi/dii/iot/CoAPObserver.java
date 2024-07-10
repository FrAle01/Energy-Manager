package com.unipi.dii.iot;


import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;


public class CoAPObserver implements Runnable {

    private CoapClient client;
    private  CoapObserveRelation relation;
    private String sensor;
    private String ipv6;
    

    public CoAPObserver(String address,String sensor) {
       
        // CoAP client constructor
        String uri = "coap://[" + address + "]:5683/"+ sensor;
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
                    json = (JSONObject) parser.parse(content);

                    String sensing =(String) json.get("sensor");
                    Double value =(Double) json.get("value");
                    String ts =(String) json.get("ts");
                    
                    Boolean added = db.insertSensorValue(sensing, ipv6, value, ts);

                    if(added){
                        System.out.println("Sensor"+sensing+"new value ("+value+") inserted");
                    }else{
                        System.err.println("Failed to insert new value");
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

