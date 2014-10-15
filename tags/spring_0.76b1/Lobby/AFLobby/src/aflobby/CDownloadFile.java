/*
 * CDownloadFile.java
 *
 * Created on 14 September 2007, 06:04
 */

package aflobby;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.List;
import javax.swing.SwingUtilities;
import javax.swing.SwingWorker;

/**
 *
 * @author  AF-StandardUsr
 */
public class CDownloadFile extends javax.swing.JFrame {

    private String address = "";
    private String targetfile = "";

    /** Creates new form CDownloadFile */
    public CDownloadFile(String address, String localFileName) {
        this.address = address;
        this.targetfile = localFileName;
        initComponents();
        Runnable doWorkRunnable = new Runnable() {

            public void run() {
                setVisible(false);
            }
        };
        SwingUtilities.invokeLater(doWorkRunnable);

        
    }

    private static class CDLProgressData {

        private final int percentage;
        private final String progresslabel;

        CDLProgressData(int percentage, String label) {
            this.percentage = percentage;
            this.progresslabel = label;
        }
    }

    public void StartDownload() {
        Runnable doWorkRunnable = new Runnable() {

            public void run() {
                setVisible(true);
            }
        };
        SwingUtilities.invokeLater(doWorkRunnable);
        SwingWorker worker = new SwingWorker<Void, CDLProgressData>() {

            @Override
            public Void doInBackground() {
                OutputStream out = null;
                URLConnection conn = null;
                InputStream in = null;
                try {
                    URL url = new URL(address);
                    out = new BufferedOutputStream(new FileOutputStream(targetfile));
                    conn = url.openConnection();
                    in = conn.getInputStream();
                    byte[] buffer = new byte[1024];
                    int numRead;
                    long numWritten = 0;
                    while ((numRead = in.read(buffer)) != -1) {
                        out.write(buffer, 0, numRead);
                        numWritten += numRead;
                        int prog = -1;
                        if(conn.getContentLength() != -1){
                            prog = (int) numWritten/(conn.getContentLength()/100);
                        }
                        publish(new CDLProgressData(prog, "" + (numWritten/1024)+"KB of "+ (conn.getContentLength()/1024)+"KB"));
                    }
                    //System.out.println(targetfile + "\t" + numWritten);
                } catch (Exception exception) {
                    exception.printStackTrace();
                } finally {
                    try {
                        if (in != null) {
                            in.close();
                        }
                        if (out != null) {
                            out.close();
                        }
                    } catch (IOException ioe) {
                    }
                }
                return null;
            }

            @Override
            protected void process(List<CDLProgressData> pairs) {
                CDLProgressData pair = pairs.get(pairs.size() - 1);
                DLprogresslabel.setText("Downloading: "+pair.progresslabel);
                if(pair.percentage==-1){
                    DLprogressbar.setIndeterminate(true);
                }else{
                    DLprogressbar.setIndeterminate(false);
                    DLprogressbar.setValue(pair.percentage);
                }
                
            }

            @Override
            public void done() {
                //Remove the "Loading images" label.
                DLprogresslabel.setText("download complete");
//                try {
//                    get();
//                } catch (InterruptedException ignore) {
//                } catch (java.util.concurrent.ExecutionException e) {
//                }
            }
        };
        worker.execute();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        DLprogressbar = new javax.swing.JProgressBar();
        DLprogresslabel = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Downloading");
        setResizable(false);

        DLprogressbar.setIndeterminate(true);

        DLprogresslabel.setText("Downloading X of YMB at ZKBps");

        jLabel2.setFont(new java.awt.Font("Arial", 1, 18));
        jLabel2.setText("Downloading");

        jLabel3.setText("from: "+address);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel2, javax.swing.GroupLayout.PREFERRED_SIZE, 232, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(DLprogressbar, javax.swing.GroupLayout.DEFAULT_SIZE, 511, Short.MAX_VALUE)
                    .addComponent(DLprogresslabel, javax.swing.GroupLayout.PREFERRED_SIZE, 204, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel3, javax.swing.GroupLayout.DEFAULT_SIZE, 511, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel2, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel3)
                .addGap(9, 9, 9)
                .addComponent(DLprogressbar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(DLprogresslabel)
                .addContainerGap(20, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JProgressBar DLprogressbar;
    private javax.swing.JLabel DLprogresslabel;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    // End of variables declaration//GEN-END:variables
}