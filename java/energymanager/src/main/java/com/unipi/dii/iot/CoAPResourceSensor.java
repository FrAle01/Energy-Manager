package com.unipi.dii.iot;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.server.resources.CoapExchange;

public class CoAPResourceSensor extends CoapResource{

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

    }
}
