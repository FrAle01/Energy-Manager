package com.unipi.dii.iot;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class DatabaseAccess {
    
    static final String URL = "jdbc:mysql://localhost:3306/energymanager";
    static final String USER = "root";
    static  final String PASSWORD = "password";

    private static Connection dbConnect() throws SQLException {
        return DriverManager.getConnection(URL, USER, PASSWORD);
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
}
