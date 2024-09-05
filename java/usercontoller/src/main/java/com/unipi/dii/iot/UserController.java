package com.unipi.dii.iot;

import java.util.List;
import java.util.Scanner;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;

import java.io.IOException;

import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

import com.unipi.dii.iot.DatabaseAccess.SensorIp;

public class UserController {

    public static void main(String[] args) {
        
        DatabaseAccess db = new DatabaseAccess();
        Scanner scanner = new Scanner(System.in);

        while (true) { 
            printCommands();

            try {

                int command = scanner.nextInt();
                scanner.nextLine();
                String addr = null;

                switch (command) {
                    case 1:

                        addr = db.getAddress("actuator", "inverter");

                        if(addr == null){
                            
                            System.out.println("No actuator found: " + addr +"\n");
                            break;
                        }
                        
                        String uri = "coap://[" + addr + "]:5683/batterylimit";
                        CoapClient client = new CoapClient(uri);
                        CoapResponse response = client.get();

                        if(response == null){
                            System.out.print("Error reading battery limit resource\n");
                        }else{
                            JSONObject presentLimit = null;
                            JSONParser parser = new JSONParser();
                            presentLimit = (JSONObject) parser.parse(response.getResponseText());
                            Double limit = (Double) presentLimit.get("limit");
                            System.out.print("\n\nCurrent battery limit setted at " + limit + "%\n");

                        }

                        System.out.print("\nTo set new limit digit the value desired(0-100): ");
                        int batteryLimit = scanner.nextInt();

                        if(batteryLimit < 0 || batteryLimit > 100){
                            System.out.println("\nError percentage must be in range 0-100\n");
                            break;
                        }

                        String payload = Integer.toString(batteryLimit);
                        CoapResponse response2 = client.post(payload, MediaTypeRegistry.TEXT_PLAIN);



                        if (response2 != null) {
                            System.out.println(response2.getResponseText()+"%\n");
                        } else {
                            System.out.println("No response from resource.\n");
                        }

                        break;

                    case 2:
                        System.out.print("Strarting observation of energy flow... \n");

                        addr = db.getAddress("actuator", "inverter");

                        if(addr == null)
                        {
                            System.out.println("No actuator found: " + addr);
                            break;
                        }
                        
                        
                        
                        final FlowObserver observer = new FlowObserver(addr, "energyflow");
                        observer.startObserving();

                        System.out.print("Press any key to interrupt monitoring\n");
                        System.out.print("| --- PRODUCTION ---- HOME ---- BATTERY ---- GRID -- | /    TIMESTAMP    /\n");
                        System.out.print("__________________________________________________________________________\n");

                        Boolean running = true;

                        while(running){
                            try {
                                if (System.in.available() > 0) { // got some keyboard input
                                    observer.stopObserving();   // stop the observer
                                    running = false; // stop the loop                                

                                }
                            } catch (IOException e) {
                                System.out.println("Error reading keyboard input.\n");
                                e.printStackTrace();
                            }
                        }
                        scanner.nextLine();

                        break;

                    case 3:

                        List<SensorIp> onlineSensors = db.getSensorAddress();
                
                        for(SensorIp sensor : onlineSensors){
                            System.out.print("Sensor "+sensor.name +": ONLINE\n");
                        }
                        if(onlineSensors.size() < 4){
                            Integer offlineSensorNum = 4 - onlineSensors.size();
                            System.out.print("Sensors OFFLINE: "+ offlineSensorNum + "\n");
                        }
                        System.out.print("\n");

                        break;

                    default:
                        System.out.println("Invalid choice, input must be in range 1-4");
                        break;
                }

            } catch (Exception e) {
                System.out.println("Invalid input, input must be integer in range 1-4");
                scanner.nextLine();
            }
        }
    }

    private static void printCommands(){
        System.out.println("User Control Application");
        System.out.println("1. Set the battery percentage limit"); 
        System.out.println("2. Monitor the energy flow"); 
        System.out.println("3. Check connections"); 
        System.out.println("4. exit"); 
        System.out.print("\nPress the key corresponding to the command:");
    }
}