package com.unipi.dii.iot;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;



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
        
        String insertSQL = "INSERT INTO ipv6_addresses (address, type, name) VALUES (?, ?, ?)";

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

    public List<String> fetchAddresses() {
        List<String> ipAddresses = new ArrayList<>();
        String querySQL = "SELECT address FROM ipv6_addresses";

        try{
            Connection conn = getConnection();
            Statement stmt = conn.createStatement();
            ResultSet resultSet = stmt.executeQuery(querySQL);

            while (resultSet.next()) {
                String address = resultSet.getString("address");
                ipAddresses.add(address);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        return ipAddresses;
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
