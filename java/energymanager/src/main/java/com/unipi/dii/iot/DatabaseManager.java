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
        String sql_server_url = "jdbc:mysql://localhost:3306";
        String dbName = "energymanager";

        String createDBquery = "CREATE DATABASE IF NOT EXISTS " + dbName;

        try {
            Connection conn = DriverManager.getConnection(sql_server_url);
            Statement stmt = conn.createStatement();
            stmt.execute(createDBquery);
            System.out.println("db created");

        } catch (SQLException e) {
            System.out.println("db NOT created");
            e.printStackTrace();
        }
    }

    private static Connection getConnection() throws SQLException {
        return DriverManager.getConnection(URL, USER, PASSWORD);
    }

    public static void createAddressTable() {
        String createTable = "CREATE TABLE IF NOT EXISTS addresses (" +
                                "id INT AUTO_INCREMENT PRIMARY KEY, " +
                                "time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, " +
                                "address VARCHAR(89) NOT NULL, " +
                                "type VARCHAR(80) NOT NULL," + 
                                "name VARCHAR(80) NOT NULL)";

        try (Connection conn = getConnection();
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
            Connection conn = getConnection();
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            pstmt.setString(1, address);
            pstmt.setString(2, type);
            pstmt.setString(3, name);
            pstmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void createSensorTable(String sensor, String address){
        address = address.replace(":", "");

        String tableName = sensor.toLowerCase() + "_" + address; 

        String createTableSQL = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
            + "id INT AUTO_INCREMENT, " 
            + "timestamp TIMESTAMP DEFAULT NULL, "
            + "sensor VARCHAR(50) NOT NULL, "
            + "value DOUBLE NOT NULL" 
            + "PRIMARY KEY (id)) ";

        try{
            Connection conn = getConnection();
            Statement stmt = conn.createStatement();
            stmt.execute(createTableSQL);

        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public boolean elementRegistered(String element, String address){
        String querySQL =   "SELECT *" +
                            "FROM addresses" +
                            "WHERE name = ? AND address = ?";

        try{
            Connection conn = getConnection();
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

    public boolean allSensorsOnline(){
        String querySQL =   "SELECT *" +
                            "FROM addresses" +
                            "WHERE type = 'sensor'";

        int sensorsOnline = 0;
        try{
            Connection conn = getConnection();
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

    public static void deleteDB() {
        String deleteDB = "DROP DATABASE IF EXISTS energymanager";

        try{
            Connection conn = getConnection();
            Statement stmt = conn.createStatement();
            stmt.execute(deleteDB);
            System.out.println("Database deleted successfully.");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

}
