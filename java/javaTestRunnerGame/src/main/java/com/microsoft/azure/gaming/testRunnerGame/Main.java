// Copyright (C) Microsoft Corporation. All rights reserved.

package com.microsoft.azure.gaming.testRunnerGame;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.microsoft.azure.gaming.*;

import javax.xml.bind.DatatypeConverter;
import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * Simple app that lets us test the various Java GSDK methods
 */
public class Main {
    private static final int ListeningPort = 3600;
    private static final String AssetFileTextPath = "/data/Assets/testassetfile.txt";
    private static final String AssetFileTarPath = "/data/AssetsTar/testassetfile.txt";
    private static final String AssetFileTarGzPath = "/data/AssetsTarGz/testassetfile.txt";
    private static String assetFileText = "";
    private static String assetFileTar = "";
    private static String assetFileTarGz = "";
    private static ServerSocket server;
    private static boolean isActivated = false;
    private static boolean isShutdown = false;
    private static boolean delayShutdown = false;
    private static String logsDirectory = "";
    private static String sharedContentDirectory = "";
    private static String certificateDirectory = "";
    private static String testCertificate = "";
    private static ArrayList<ConnectedPlayer> players = new ArrayList<ConnectedPlayer>();
    private static int requestCount = 0;
    private static ZonedDateTime nextMaintenance = null;

    private static final String RESPONSE_HEADERS = "HTTP/1.1 200 OK\r\n" +
                                                   "Content-Type: application/json\r\n" +
                                                   "Content-Length: ";
    private static final String RESPONSE_END_OF_HEADERS = "\r\n\r\n";

    private static void onShutdown()
    {
        System.out.println("Shutting down...");
        isShutdown = true;

        if (!delayShutdown)
        {
            try {
                server.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private static GameHostHealth getGameHealth()
    {
        // Flip so we can test both
        return requestCount % 2 == 0? GameHostHealth.Unhealthy : GameHostHealth.Healthy;
    }

    private static void onMaintenanceScheduled(ZonedDateTime time)
    {
        nextMaintenance = time;
    }

    public static void main(String[] args){
        try
        {
            server = new ServerSocket(ListeningPort);

            GameserverSDK.registerShutdownCallback(Main::onShutdown);
            GameserverSDK.registerHealthCallback(Main::getGameHealth);
            GameserverSDK.registerMaintenanceCallback(Main::onMaintenanceScheduled);

            // Read our asset files
            assetFileText = getAssetText(AssetFileTextPath);
            assetFileTar = getAssetText(AssetFileTarPath);
            assetFileTarGz = getAssetText(AssetFileTarGzPath);

            // Get the various folder names.
            logsDirectory = GameserverSDK.getLogsDirectory();
            sharedContentDirectory = GameserverSDK.getSharedContentDirectory();
            certificateDirectory = GameserverSDK.getCertificateDirectory();

            // Get our cert
            testCertificate = getTestCertificateThumbprint();


            Thread t = new Thread(() -> processRequests());
            t.start();

            GameserverSDK.readyForPlayers();
            isActivated = true;
        }
        catch(Exception e)
        {
            System.out.println(e.toString());
        }
    }

    public static void processRequests()
    {
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        while (!server.isClosed())
        {
            try (Socket socket = server.accept())
            {
                System.out.println("HTTP: Received");
                BufferedReader inputReader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                String line;
                while (!(line = inputReader.readLine()).equals(""))
                {
                    System.out.println(line);
                }

                Map<String, String> config = null;

                // For each request, "add" a connected player
                players.add(new ConnectedPlayer("gamer" + requestCount));
                requestCount++;
                GameserverSDK.updateConnectedPlayers(players);

                config = Optional.ofNullable(GameserverSDK.getConfigSettings()).orElse(new HashMap<String, String>());

                // First, check if we need to delay shutdown for testing
                if (config.containsKey(GameserverSDK.SESSION_COOKIE_KEY) && config.get(GameserverSDK.SESSION_COOKIE_KEY).equalsIgnoreCase("delayshutdown"))
                {
                    delayShutdown = true;
                }

                config.put("isActivated", String.valueOf(isActivated));
                config.put("isShutdown", String.valueOf(isShutdown));
                config.put("assetFileText", assetFileText);
                config.put("assetFileTar", assetFileTar);
                config.put("assetFileTarGz", assetFileTarGz);
                config.put("logFolder", logsDirectory);
                config.put("sharedContentFolder", sharedContentDirectory);
                config.put("certificateFolder", certificateDirectory);
                config.put("testCertificate", testCertificate);

                if (nextMaintenance != null) {
                    config.put("nextMaintenance", nextMaintenance.withZoneSameInstant(ZoneId.systemDefault()).format(DateTimeFormatter.ISO_OFFSET_DATE_TIME));
                }

                String content = gson.toJson(config);

                String httpResponse = RESPONSE_HEADERS + content.length() + RESPONSE_END_OF_HEADERS + content;

                OutputStream output = socket.getOutputStream();
                output.write(httpResponse.getBytes("UTF-8"));
                output.flush();
                inputReader.close(); // also closes the socket, so we want to do this after writing our response

                // Once we're shut down, return a response one more time (so the tests can
                // verify the isShutdown field) and then actually terminate
                if (isShutdown)
                {
                    server.close();
                }
            }
            catch (SocketException e)
            {
                // Nothing to do here, just means we closed the socket because we were asked to shutdown
            }
            catch(Exception e)
            {
                System.err.println(e.toString());
                e.printStackTrace();
            }
        }
    }

    private static String getAssetText(String assetFilePath)
    {
        try {
            Path assetPath = Paths.get(assetFilePath);
            if (Files.exists(assetPath)) {
                return new String(Files.readAllBytes(assetPath), "UTF-8");
            }
        }
        catch (IOException e)
        {
            System.out.println(e.toString());
        }
        return "";
    }

    private static String getTestCertificateThumbprint() throws IOException, CertificateException, NoSuchAlgorithmException {
        if (certificateDirectory != null && !certificateDirectory.isEmpty()) {
            File certFile = new File(certificateDirectory, "LinuxRunnerTestCert.pem");

            if (certFile.exists()) {
                byte[] pemData = Files.readAllBytes(certFile.toPath());
                byte[] certBytes = parseDERFromPEM(pemData, "-----BEGIN CERTIFICATE-----", "-----END CERTIFICATE-----");
                X509Certificate cert = getCertificateFromDER(certBytes);
                return DatatypeConverter.printHexBinary(MessageDigest.getInstance("SHA-1").digest(cert.getEncoded())).toLowerCase();
            }
        }

        return "";
    }

    private static byte[] parseDERFromPEM(byte[] pem, String begin, String end)
    {
        String pemString = new String(pem, Charset.forName("UTF-8"));
        int beginIndex = pemString.indexOf(begin) + begin.length();
        int endIndex = pemString.indexOf(end);
        String data = pemString.substring(beginIndex, endIndex);
        return DatatypeConverter.parseBase64Binary(data);
    }

    private static X509Certificate getCertificateFromDER(byte[] certBytes) throws CertificateException
    {
        CertificateFactory factory = CertificateFactory.getInstance("X.509");
        return (X509Certificate)factory.generateCertificate(new ByteArrayInputStream(certBytes));
    }
}
