package com.unipi.dii.iot;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;



public class DatabaseManager{

    static final String URL = "jdbc:mysql://localhost:3306/energymanager";
    static final String USER = "root";
    static  final String PASSWORD = "password";

    public static void createDB(){
        String sql_server_url = "jdbc:mysql://localhost:3306/";
        String dbName = "energymanager";

        String createDBquery = "CREATE DATABASE IF NOT EXISTS " + dbName;

        try {
            Connection conn = DriverManager.getConnection(sql_server_url, USER, PASSWORD);
            Statement stmt = conn.createStatement();
            stmt.execute(createDBquery);
            System.out.println("db created");

        } catch (SQLException e) {
            System.out.println("db NOT created");
            e.printStackTrace();
        }
    }

    private static Connection dbConnect() throws SQLException {
        return DriverManager.getConnection(URL, USER, PASSWORD);
    }

    public static void createAddressTable() {
        String createTable = "CREATE TABLE IF NOT EXISTS addresses (" +
                                "id INT AUTO_INCREMENT PRIMARY KEY, " +
                                "time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, " +
                                "address VARCHAR(89) NOT NULL, " +
                                "type VARCHAR(80) NOT NULL," + 
                                "name VARCHAR(80) NOT NULL)";

        try (Connection conn = dbConnect();
             Statement stmt = conn.createStatement()) {
            stmt.execute(createTable);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void insertAddress(String address, String type, String name) {
        createAddressTable();
        
        String insertSQL = "INSERT INTO addresses (address, type, name) VALUES (?, ?, ?)";

        try {
            Connection conn = dbConnect();
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            pstmt.setString(1, address);
            pstmt.setString(2, type);
            pstmt.setString(3, name);
            pstmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void deleteAddress(String address, String name) {
        createAddressTable();
        
        String deleteSQL = "DELETE FROM addresses WHERE address = ? AND name = ?";

        try {
            Connection conn = dbConnect();
            PreparedStatement pstmt = conn.prepareStatement(deleteSQL);
            pstmt.setString(1, address);
            pstmt.setString(2, name);
            pstmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void createSensorTable(String sensor, String address){
        address = address.replace(":", "");

        String tableName = sensor.toLowerCase() + "_" + address; 

        String createTableSQL = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                                    + "id INT AUTO_INCREMENT PRIMARY KEY, " 
                                    + "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                    + "sensor VARCHAR(50) NOT NULL, "
                                    + "value DOUBLE NOT NULL)"; 

        try{
            Connection conn = dbConnect();
            Statement stmt = conn.createStatement();
            stmt.execute(createTableSQL);

        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void createActuatorTable(String actuator, String address){
        address = address.replace(":", "");

        String tableName = actuator.toLowerCase() + "_" + address; 

        String createTableSQL = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                                    + "id INT AUTO_INCREMENT PRIMARY KEY, " 
                                    + "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                    + "produced DOUBLE NOT NULL," 
                                    + "home DOUBLE NOT NULL," 
                                    + "battery DOUBLE NOT NULL," 
                                    + "grid DOUBLE NOT NULL) ";

        try{
            Connection conn = dbConnect();
            Statement stmt = conn.createStatement();
            stmt.execute(createTableSQL);

        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public boolean elementRegistered(String element, String address){
        String querySQL =   "SELECT *" +
                            " FROM addresses" +
                            " WHERE name = ? AND address = ?";

        try{
            Connection conn = dbConnect();
            PreparedStatement pstmt = conn.prepareStatement(querySQL);
            pstmt.setString(1, element);
            pstmt.setString(2, address);
            ResultSet resultSet = pstmt.executeQuery();

            if (resultSet.next()){  // sensor already exists
                return true;
            }else{
                return false;
            }

        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }     
    }

    public boolean  insertSensorValue(String sensor, String address, Double value){

        address = address.replace(":", "");
        String tableName = sensor +"_"+  address;
        
        String insertSQL = "INSERT INTO "+tableName+ " (sensor, value) VALUES (?, ?)";

        try {
            Connection conn = dbConnect();
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            pstmt.setString(1, sensor);
            pstmt.setDouble(2, value);
            pstmt.executeUpdate();
            return true;

        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean  insertFlowValues(String actuator, String address, Double produced, Double home, Double battery, Double grid){

        address = address.replace(":", "");
        String tableName = actuator +"_"+  address;
        
        String insertSQL = "INSERT INTO "+tableName+ " (produced, home, battery, grid) VALUES (?, ?, ?, ?)";

        try {
            Connection conn = dbConnect();
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            pstmt.setDouble(1, produced);
            pstmt.setDouble(2, home);
            pstmt.setDouble(3, battery);
            pstmt.setDouble(4, grid);
            pstmt.executeUpdate();
            return true;

        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean allSensorsOnline(){
        String querySQL =   "SELECT *" +
                            " FROM addresses" +
                            " WHERE type = 'sensor'";

        int sensorsOnline = 0;
        try{
            Connection conn = dbConnect();
            PreparedStatement pstmt = conn.prepareStatement(querySQL);
            ResultSet resultSet = pstmt.executeQuery();

            if (resultSet.next()){  // sensor already exists
                do {
                    sensorsOnline++;
                } while (resultSet.next());

                return (sensorsOnline >= 4);
                 
            }else{
                return false;
            }

        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }   
    }

    public String getAddress(String type, String name){
        String address;
        String querySQL =   "SELECT address" +
                            " FROM addresses" +
                            " WHERE type = ? AND name = ?";
        
        try{
            Connection conn = dbConnect();
            PreparedStatement pstmt = conn.prepareStatement(querySQL);
            pstmt.setString(1, type);
            pstmt.setString(2, name);

            ResultSet resultSet = pstmt.executeQuery();

            if(resultSet.next()){
                address = resultSet.getString("address");
            }else{
                address = null;
            }
        } catch (SQLException e) {
            e.printStackTrace();
            address = null;
        }
        return address;
    }

    public static void deleteDB() {
        String deleteDB = "DROP DATABASE IF EXISTS energymanager";
        String sql_server_url = "jdbc:mysql://localhost:3306/";


        try{
            Connection conn = DriverManager.getConnection(sql_server_url, USER, PASSWORD);
            Statement stmt = conn.createStatement();
            stmt.execute(deleteDB);
            System.out.println("Database deleted successfully.");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

}
