using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using System.IO;
using System.Timers;
using System.Windows.Forms;
using System.ComponentModel;
using Springie.Client;
using Springie.SpringNamespace;
using Springie.AutoHostNamespace;
using System.Diagnostics;

namespace Springie
{
  public class Main
  {
    public MainConfig config;
    public const string ConfigMain = "main.xml";
    System.Timers.Timer recon;

    Stats stats;
    public Stats Stats {
      get { return stats; }
    }

    TasClient tas;
    public TasClient Tas
    {
      get
      {
        return tas;
      }
    }
    Spring spring;
    public Spring Spring
    {
      get
      {
        return spring;
      }
    }
    AutoHost autoHost;
    public AutoHost AutoHost
    {
      get
      {
        return autoHost;
      }
    }

    private AutoUpdater autoUpdater;

    public void LoadConfig()
    {
      config = new MainConfig();
      if (File.Exists(ConfigMain)) {
        XmlSerializer s = new XmlSerializer(config.GetType());
        StreamReader r = File.OpenText(ConfigMain);
        config = (MainConfig)s.Deserialize(r);
        r.Close();
      }
    }

    public void SaveConfig()
    {

      XmlSerializer s = new XmlSerializer(config.GetType());
      FileStream f = File.OpenWrite(Application.StartupPath + '/' + ConfigMain);
      f.SetLength(0);
      s.Serialize(f, config);
      f.Close();

    }

    public Main()
    {
      LoadConfig();
      SaveConfig();
    }

    public bool Start()
    {
      if (config.AttemptToRecconnect) {
        recon = new System.Timers.Timer(config.AttemptReconnectInterval * 1000);
        recon.Elapsed += new ElapsedEventHandler(recon_Elapsed);
      }

      recon.Enabled = false;

      try {
        spring = new Spring(config.SpringPath);
      } catch {
        MessageBox.Show("Spring not found in " + config.SpringPath, "error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        OpenFileDialog od = new OpenFileDialog();
        od.FileName = Spring.ExecutableName;
        od.DefaultExt = Path.GetExtension(Spring.ExecutableName);
        od.InitialDirectory = config.SpringPath;
        od.Title = "Please select your spring installation";
        od.RestoreDirectory = true;
        od.CheckFileExists = true;
        od.CheckPathExists = true;
        od.AddExtension = true;
        od.Filter = "Executable (*.exe)|*.exe";
        DialogResult dr = od.ShowDialog();
        if (dr == DialogResult.OK) {
          config.SpringPath = Path.GetDirectoryName(od.FileName);
          SaveConfig();
          try {
            spring = new Spring(config.SpringPath);
          } catch (Exception e) {
            MessageBox.Show(e.ToString(), "Error while checking spring, exiting");
            Application.Exit();
            return false;
          }
        } else {
          MessageBox.Show("Spring not found, exiting");
          Application.Exit();
          return false;
        }
      }


      tas = new TasClient();
      tas.ConnectionLost += new EventHandler<TasEventArgs>(tas_ConnectionLost);
      tas.Connected += new EventHandler<TasEventArgs>(tas_Connected);
      tas.LoginDenied += new EventHandler<TasEventArgs>(tas_LoginDenied);
      tas.LoginAccepted += new EventHandler<TasEventArgs>(tas_LoginAccepted);
      tas.Said += new EventHandler<TasSayEventArgs>(tas_Said);
      tas.MyStatusChangedToInGame += new EventHandler<TasEventArgs>(tas_MyStatusChangedToInGame);
      spring.SpringExited += new EventHandler(spring_SpringExited);
      spring.PlayerSaid += new EventHandler<SpringLogEventArgs>(spring_PlayerSaid);
      autoHost = new AutoHost(tas, spring, null);
      autoUpdater = new AutoUpdater(spring, tas);

      if (config.StatsEnabled) stats = new Stats(tas, spring);
      try {
        tas.Connect(config.ServerHost, config.ServerPort);
      } catch {
        recon.Start();
      }
      return true;
    }


    public void Stop() {
      tas.ConnectionLost -= tas_ConnectionLost;
      tas.Disconnect();
    }

    void tas_MyStatusChangedToInGame(object sender, TasEventArgs e)
    {
      spring.StartGame(tas.GetBattle());
    }

    void tas_Said(object sender, TasSayEventArgs e)
    {
      if (config.RedirectGameChat && e.Place == TasSayEventArgs.Places.Battle && e.Origin == TasSayEventArgs.Origins.Player && e.UserName != tas.UserName && e.IsEmote == false) {
        spring.SayGame("[" + e.UserName + "]" + e.Text);
      }
    }

    void spring_PlayerSaid(object sender, SpringLogEventArgs e)
    {
      tas.GameSaid(e.Username, e.Line);
      if (config.RedirectGameChat && e.Username != tas.UserName && !e.Line.StartsWith("Allies:") && !e.Line.StartsWith("Spectators:")) {
        tas.Say(TasClient.SayPlace.Battle, "", "[" + e.Username + "]" + e.Line, false);
      }
    }

    public void ReLogin()
    {
      tas_Connected(this, new TasEventArgs());
    }

    void tas_LoginDenied(object sender, TasEventArgs e)
    {
      //MessageBox.Show(e.ServerParams[0], "Login failed", MessageBoxButtons.OK, MessageBoxIcon.Warning);
      Program.formMain.GetNewLogPass();
    }

    void spring_SpringExited(object sender, EventArgs e)
    {
      if (tas != null) tas.ChangeMyStatus(false, false);
    }


    // login accepted - join channels
    void tas_LoginAccepted(object sender, TasEventArgs e)
    {
      for (int i = 0; i < config.JoinChannels.Length; ++i) {
        tas.JoinChannel(config.JoinChannels[i]);
      }
      autoHost.Start(null, null);
    }

    // im connected, let's login
    void tas_Connected(object sender, TasEventArgs e)
    {
      tas.Login(config.AccountName, config.AccountPassword, MainConfig.SpringieVersion);
    }


    void recon_Elapsed(object sender, ElapsedEventArgs e)
    {
      recon.Stop();
      try {
        tas.Connect(config.ServerHost, config.ServerPort);
      } catch {
        recon.Start();
      }
    }

    void tas_ConnectionLost(object sender, TasEventArgs e)
    {
      autoHost.Stop();
      recon.Start(); 
    }
  }
}
