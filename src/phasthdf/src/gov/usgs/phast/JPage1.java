/*
 * JPage1.java
 *
 * Created on July 24, 2002, 3:42 PM
 */

package gov.usgs.phast;

/**
 *
 * @author  charlton
 */
public class JPage1 extends javax.swing.JPanel implements java.beans.VetoableChangeListener {
   
    /** Creates new form JPage1 */
    public JPage1() {
        initComponents();
        jFileChooser = new javax.swing.JFileChooser();
        jFileChooser.addChoosableFileFilter( new javax.swing.filechooser.FileFilter() {
            public boolean accept(java.io.File f) {
                // if (f.isDirectory()) return true;
                return (f.isDirectory() || f.getName().endsWith(".h5"));
            }
            public String getDescription() {
                return "Phast HDF files";
            }
        }
        );
        jFileChooser.setFileSelectionMode(javax.swing.JFileChooser.FILES_ONLY);
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    private void initComponents() {//GEN-BEGIN:initComponents
        java.awt.GridBagConstraints gridBagConstraints;

        jFileChooser = new javax.swing.JFileChooser();
        jPanel1 = new javax.swing.JPanel();
        fileLabel = new javax.swing.JLabel();
        fileTextField = new javax.swing.JTextField();
        browseButton = new javax.swing.JButton();

        setLayout(new java.awt.BorderLayout());

        jPanel1.setLayout(new java.awt.GridBagLayout());

        fileLabel.setFont(new java.awt.Font("Dialog", 0, 11));
        fileLabel.setText("PHAST HDF file:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 0.5;
        gridBagConstraints.insets = new java.awt.Insets(5, 20, 2, 20);
        jPanel1.add(fileLabel, gridBagConstraints);

        fileTextField.setFont(new java.awt.Font("Dialog", 0, 11));
        fileTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                fileTextFieldActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(5, 20, 2, 20);
        jPanel1.add(fileTextField, gridBagConstraints);

        browseButton.setFont(new java.awt.Font("Dialog", 0, 11));
        browseButton.setMnemonic('r');
        browseButton.setText("Browse...");
        browseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                browseButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.EAST;
        gridBagConstraints.insets = new java.awt.Insets(5, 20, 2, 20);
        jPanel1.add(browseButton, gridBagConstraints);

        add(jPanel1, java.awt.BorderLayout.CENTER);

    }//GEN-END:initComponents

    private void browseButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_browseButtonActionPerformed
        // Add your handling code here:        
        try {
            String str = fileTextField.getText();
            String file = str.trim();
            if (file != null && file.length() > 0) {
                jFileChooser.setCurrentDirectory(new java.io.File(file));
            }
            else {
                jFileChooser.setCurrentDirectory(null);
            }
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
        }
        if(jFileChooser.showOpenDialog(this) == javax.swing.JFileChooser.APPROVE_OPTION) {
            fileTextField.setText(jFileChooser.getSelectedFile().getAbsolutePath());
        }
        
    }//GEN-LAST:event_browseButtonActionPerformed

    private void fileTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_fileTextFieldActionPerformed
        // Add your handling code here:
    }//GEN-LAST:event_fileTextFieldActionPerformed

    public void vetoableChange(java.beans.PropertyChangeEvent propertyChangeEvent) throws java.beans.PropertyVetoException {
        /**
        if (propertyChangeEvent.getSource() instanceof JWizardPanel) {
            if (propertyChangeEvent.getPropertyName().equals(JWizardPanel.PAGE_CHANGING_PROPERTY)) {
                if (propertyChangeEvent.getOldValue().equals(this)) {
                    if (fileTextField.getText().length() == 0) {
                        javax.swing.JOptionPane.showMessageDialog(this, "Please enter a complete path and filename", "Import Favorites", javax.swing.JOptionPane.ERROR_MESSAGE);
                        throw new java.beans.PropertyVetoException("No file name", propertyChangeEvent);
                    }
                    java.io.File file = new java.io.File(fileTextField.getText());
                    if (!file.exists()) {
                        /// "Cannot find " + fileTestField.getText() + ".\n Please choose another file."
                        javax.swing.JOptionPane.showMessageDialog(this, "Cannot find " + fileTextField.getText() + ".\n Please choose another file.",
                        "Import Favorites", javax.swing.JOptionPane.ERROR_MESSAGE);
                        throw new java.beans.PropertyVetoException("Bad file name", propertyChangeEvent);
                    }
                    if (!file.isFile()) {
                        javax.swing.JOptionPane.showMessageDialog(this, "Please enter a complete path and filename", "Import Favorites", javax.swing.JOptionPane.ERROR_MESSAGE);
                        throw new java.beans.PropertyVetoException("Bad file name", propertyChangeEvent);
                    }
                }
            }
        }
        **/
    }
    
    public String getText() {
        return fileTextField.getText();
    }    
    public void setText(String text) {
        fileTextField.setText(text);
    }    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel jPanel1;
    private javax.swing.JTextField fileTextField;
    private javax.swing.JButton browseButton;
    private javax.swing.JLabel fileLabel;
    private javax.swing.JFileChooser jFileChooser;
    // End of variables declaration//GEN-END:variables
    
}
