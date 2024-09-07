package com.unipi.dii.iot;

import java.time.LocalDateTime;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

public class FlowObserver implements Runnable{
    
    private CoapClient client;
    private  CoapObserveRelation relation;
    private String resource;
    private String ipv6;
    

    public FlowObserver(String address,String resource) {
       
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
                
                String content = response.getResponseText();
                JSONObject json= null;

                if(resource.equals("energyflow")){
                    try{
                        JSONParser parser = new JSONParser();
                        json = (JSONObject) parser.parse(content);
    
                        String str_home =(String) json.get("h");
                        String str_battery =(String) json.get("b");
                        String str_grid =(String) json.get("g");

                        String ts = LocalDateTime.now().toString();

                        Double to_home = Double.parseDouble(str_home);
                        Double to_battery = Double.parseDouble(str_battery);
                        Double to_grid = Double.parseDouble(str_grid);
                        
                        Double produced =to_home + to_battery + to_grid;
    
                        if(!produced.isNaN() && !to_home.isNaN() && !to_battery.isNaN() && !to_grid.isNaN() && !ts.isEmpty()){
                            System.out.println("| -- "+produced+" ---- "+ to_home + " ---- "+  to_battery +" ---- "+ to_grid + "-- |"+ " / "+ts+" /\n");
                        }else{
                            System.err.println("Failed to receive new flow");
                        }
    
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }else{

                    System.err.println("Trying to observe wrong resource");

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
