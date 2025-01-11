package org.example;

import com.influxdb.client.InfluxDBClient;
import com.influxdb.client.InfluxDBClientFactory;
import com.influxdb.client.QueryApi;
import com.influxdb.query.FluxTable;
import com.influxdb.query.FluxRecord;

import java.util.List;

public class InfluxDBReader {
    public static void main(String[] args) {
        // Deine InfluxDB Cloud 2.0 Zugangsdaten
        String url = "https://eu-central-1-1.aws.cloud2.influxdata.com";
        String token = "-LYKm4RBjIfahFiBXb5_TJ-lXclI35tyrfDC1uRr20KVq1Hl6TiYupEvuvhcbTF1_IMX5_qaZPDqUJgdC9OcVA==";
        String org = "edCorp";
        String bucket = "AndroidDB";

        // Verbindung zur InfluxDB herstellen
        InfluxDBClient client = InfluxDBClientFactory.create(url, token.toCharArray());
        QueryApi queryApi = client.getQueryApi();

        // Thread erstellen
        Thread dataFetcherThread = new Thread(() -> {
            while (true) {
                try {
                    String fluxQuery = String.format(
                            "from(bucket:\"%s\") " +
                                    "|> range(start: -1h) " + // Daten aus der letzten Stunde
                                    "|> filter(fn: (r) => r._measurement == \"humidity\" or r._measurement == \"temperature\" or r._measurement == \"soil_moisture\") " +
                                    "|> sort(columns: [\"_time\"], desc: true) " + // Neueste zuerst
                                    "|> limit(n: 1)",                              // Nur den letzten Wert
                            bucket);

                    List<FluxTable> tables = queryApi.query(fluxQuery, org);

                    System.out.println("\n--- Neuste Messwerte ---");
                    for (FluxTable table : tables) {
                        for (FluxRecord record : table.getRecords()) {
                            System.out.println("Time: " + record.getTime() +
                                    ", Measurement: " + record.getMeasurement() +
                                    ", Field: " + record.getField() +
                                    ", Value: " + record.getValue());
                        }
                    }

                    // 10 Sekunden warten, bevor die Abfrage erneut ausgeführt wird
                    Thread.sleep(10_000);
                } catch (Exception e) {
                    e.printStackTrace();
                    break;
                }
            }
        });

        dataFetcherThread.start();

        try {
            dataFetcherThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            client.close();
        }
    }
}