using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Markup;
using System.Windows.Media;

[assembly: AssemblyTitle("Machinecoin Midnight Setup")]
[assembly: AssemblyProduct("Machinecoin Wallet")]
[assembly: AssemblyVersion("0.17.1.6")]
[assembly: AssemblyFileVersion("0.17.1.6")]

// Presentation only: the bundled NSIS engine owns the exact install/uninstall manifest.
internal static class MidnightSetup
{
    private static bool silent, test, uninstall, running;
    private static string destination = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Programs", "Machinecoin-Midnight");
    private static string work;
    private static Window window;
    private static Grid content;
    private static TextBlock title, subtitle, step;
    private static TextBox location;
    private static CheckBox consent;
    private static Button action, cancel;
    private static int exitCode;

    [DllImport("dwmapi.dll")]
    private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attribute, ref int value, int size);

    [STAThread]
    public static int Main(string[] args)
    {
        try
        {
            bool relocate = false;
            foreach (string arg in args)
            {
                if (arg.Equals("/S", StringComparison.OrdinalIgnoreCase)) silent = true;
                else if (arg.Equals("/TEST=1", StringComparison.OrdinalIgnoreCase)) test = true;
                else if (arg.StartsWith("/D=", StringComparison.OrdinalIgnoreCase)) destination = arg.Substring(3);
                else if (arg == "--uninstall") { uninstall = true; relocate = true; }
                else if (arg.StartsWith("--uninstall-dir=", StringComparison.Ordinal)) { uninstall = true; destination = arg.Substring(16); }
                else throw new ArgumentException("Unknown setup option: " + arg);
            }
            if (relocate)
            {
                destination = AppDomain.CurrentDomain.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar);
                string temporary = NewWork();
                string copy = Path.Combine(temporary, "MachinecoinMaintenance.exe");
                File.Copy(Assembly.GetExecutingAssembly().Location, copy);
                Process.Start(new ProcessStartInfo(copy, Quote("--uninstall-dir=" + destination) + (silent ? " /S" : "")) { UseShellExecute = false });
                return 0; // Release the installed executable before its exact manifest is removed.
            }
            destination = Path.GetFullPath(destination);
            if (silent) return Execute();
            var app = new Application();
            window = (Window)XamlReader.Parse(Layout);
            content = (Grid)window.FindName("Content");
            title = (TextBlock)window.FindName("Heading");
            subtitle = (TextBlock)window.FindName("Subtitle");
            step = (TextBlock)window.FindName("Step");
            action = (Button)window.FindName("Action");
            cancel = (Button)window.FindName("Cancel");
            try { window.Icon = System.Windows.Interop.Imaging.CreateBitmapSourceFromHIcon(System.Drawing.Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location).Handle, Int32Rect.Empty, System.Windows.Media.Imaging.BitmapSizeOptions.FromEmptyOptions()); } catch { }
            window.SourceInitialized += delegate { try { int dark = 1; DwmSetWindowAttribute(new WindowInteropHelper(window).Handle, 20, ref dark, 4); } catch { } };
            window.Closing += delegate(object sender, CancelEventArgs e) { if (running) e.Cancel = true; };
            cancel.Click += delegate { window.Close(); };
            ShowChoice();
            app.Run(window);
            return exitCode;
        }
        catch (Exception e)
        {
            try { File.WriteAllText(Path.Combine(NewWork(), "setup-error.txt"), e.ToString()); } catch { }
            return 1;
        }
    }

    private static string NewWork()
    {
        string path = Path.Combine(Path.GetTempPath(), "Machinecoin-Setup-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(path);
        return path;
    }
    private static string Quote(string value)
    {
        if (value.IndexOf('"') >= 0) throw new ArgumentException("A path cannot contain quotation marks.");
        return "\"" + value.TrimEnd('\\') + "\"";
    }
    private static TextBlock Text(string value, double size, string color)
    {
        return new TextBlock { Text = value, FontSize = size, Foreground = (Brush)new BrushConverter().ConvertFromString(color), TextWrapping = TextWrapping.Wrap, Margin = new Thickness(0, 0, 0, 14) };
    }
    private static void ShowChoice()
    {
        title.Text = uninstall ? "Uninstall Machinecoin" : "Your wallet. Your machine.";
        subtitle.Text = uninstall ? "Remove the application. Keep your wallet data." : "Install the native Windows wallet with the Midnight interface.";
        step.Text = uninstall ? "MAINTENANCE" : "WINDOWS x64  /  PREVIEW 5";
        var panel = new StackPanel();
        panel.Children.Add(Text(uninstall ? "APPLICATION LOCATION" : "INSTALL LOCATION", 11, "#A7AEBB"));
        location = new TextBox { Text = destination, IsReadOnly = uninstall, FontSize = 14, Padding = new Thickness(14), Background = new SolidColorBrush(Color.FromRgb(17, 22, 32)), Foreground = Brushes.White, BorderBrush = new SolidColorBrush(Color.FromRgb(61, 70, 86)), BorderThickness = new Thickness(1), Margin = new Thickness(0, 0, 0, 18) };
        panel.Children.Add(location);
        panel.Children.Add(Text(uninstall ? "Only installed application files will be removed. Wallet databases, blockchain data and personal settings are preserved." : "Installs for your Windows account. No administrator access is needed. Your wallet will not start automatically.", 14, "#C4CAD4"));
        if (!uninstall)
        {
            var license = new Button { Content = "Read the MIT license", HorizontalAlignment = HorizontalAlignment.Left, Margin = new Thickness(0, 0, 0, 15) };
            license.Click += delegate { ShowLicense(); };
            panel.Children.Add(license);
            consent = new CheckBox { Content = "I accept the MIT license terms.", Foreground = Brushes.White, FontSize = 14, Margin = new Thickness(0, 0, 0, 12) };
            consent.Checked += delegate { action.IsEnabled = true; };
            consent.Unchecked += delegate { action.IsEnabled = false; };
            panel.Children.Add(consent);
        }
        content.Children.Add(panel);
        action.Content = uninstall ? "Uninstall" : "Install wallet";
        action.IsEnabled = uninstall;
        action.Click += Begin;
    }
    private static void ShowLicense()
    {
        var dialog = new Window { Title = "Machinecoin - MIT license", Width = 660, Height = 540, MinWidth = 500, MinHeight = 400, Owner = window, WindowStartupLocation = WindowStartupLocation.CenterOwner, Background = window.Background, Foreground = Brushes.White, FontFamily = window.FontFamily, Icon = window.Icon };
        dialog.Resources = window.Resources;
        var panel = new DockPanel { Margin = new Thickness(28) };
        var close = new Button { Content = "Close", HorizontalAlignment = HorizontalAlignment.Right, Margin = new Thickness(0, 18, 0, 0) };
        close.Click += delegate { dialog.Close(); }; DockPanel.SetDock(close, Dock.Bottom); panel.Children.Add(close);
        string license;
        using (var reader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("Machinecoin.License"))) license = reader.ReadToEnd();
        panel.Children.Add(new ScrollViewer { VerticalScrollBarVisibility = ScrollBarVisibility.Auto, Content = Text(license, 14, "#D8DDE6") });
        dialog.Content = panel;
        dialog.SourceInitialized += delegate { try { int dark = 1; DwmSetWindowAttribute(new WindowInteropHelper(dialog).Handle, 20, ref dark, 4); } catch { } };
        dialog.ShowDialog();
    }
    private static void Begin(object sender, RoutedEventArgs e)
    {
        try
        {
            if (!Path.IsPathRooted(location.Text)) throw new ArgumentException("Enter an absolute installation path.");
            destination = Path.GetFullPath(location.Text).TrimEnd(Path.DirectorySeparatorChar);
            if (destination == Path.GetPathRoot(destination).TrimEnd(Path.DirectorySeparatorChar)) throw new ArgumentException("Choose an application folder, not a drive root.");
            Quote(destination);
        }
        catch (Exception error) { subtitle.Text = error.Message; return; }
        running = true;
        action.IsEnabled = false; cancel.IsEnabled = false;
        title.Text = uninstall ? "Removing the application" : "Setting up your wallet";
        subtitle.Text = "Please keep this window open until setup finishes.";
        step.Text = "IN PROGRESS";
        content.Children.Clear();
        var panel = new StackPanel { VerticalAlignment = VerticalAlignment.Center };
        panel.Children.Add(Text(uninstall ? "Removing installed files..." : "Unpacking and installing application files...", 18, "#FFFFFF"));
        panel.Children.Add(new ProgressBar { IsIndeterminate = true, Height = 6, Foreground = (Brush)new BrushConverter().ConvertFromString("#F2B84B"), Background = (Brush)new BrushConverter().ConvertFromString("#252D3B"), BorderThickness = new Thickness(0), Margin = new Thickness(0, 10, 0, 25) });
        panel.Children.Add(Text("Your existing wallet data stays in place.", 14, "#A7AEBB"));
        content.Children.Add(panel);
        var worker = new BackgroundWorker();
        worker.DoWork += delegate(object s, DoWorkEventArgs result) { result.Result = Execute(); };
        worker.RunWorkerCompleted += delegate(object s, RunWorkerCompletedEventArgs result)
        {
            running = false;
            exitCode = result.Error == null ? (int)result.Result : 1;
            bool ok = exitCode == 0;
            title.Text = ok ? (uninstall ? "Machinecoin removed" : "Ready when you are") : "Setup could not finish";
            subtitle.Text = ok ? (uninstall ? "The application is removed. Your wallet data is preserved." : "Machinecoin is installed. You can open it from the Start menu.") : "Close any running Machinecoin applications and try again.";
            step.Text = ok ? "COMPLETE" : "NEEDS ATTENTION";
            content.Children.Clear();
            content.Children.Add(Text(ok ? (uninstall ? "Your keys, wallet databases and blockchain data were not deleted." : "Installation complete. Mining remains opt-in, and the wallet has not been started.") : "Error code: " + exitCode + (result.Error == null ? "" : "\n" + result.Error.Message) + "\nSetup files: " + work, 16, "#C4CAD4"));
            action.Click -= Begin;
            action.Content = "Finish"; action.IsEnabled = true;
            action.Click += delegate { window.Close(); };
            cancel.Visibility = Visibility.Collapsed;
        };
        worker.RunWorkerAsync();
    }
    private static int Execute()
    {
        foreach (string name in new[] { "machinecoin-qt", "machinecoind", "machinecoin-cli" })
        {
            foreach (Process candidate in Process.GetProcessesByName(name))
            {
                using (candidate)
                {
                    string executable;
                    try { executable = candidate.MainModule.FileName; }
                    catch (Win32Exception) { continue; }
                    catch (InvalidOperationException) { continue; }
                    if (String.Equals(Path.GetDirectoryName(executable).TrimEnd(Path.DirectorySeparatorChar), destination.TrimEnd(Path.DirectorySeparatorChar), StringComparison.OrdinalIgnoreCase))
                        throw new InvalidOperationException("Close the running Machinecoin application before changing this installation.");
                }
            }
        }
        work = NewWork();
        string engine = Path.Combine(work, uninstall ? "remove.exe" : "install.exe");
        if (uninstall)
        {
            string original = Path.Combine(destination, "Uninstall.exe");
            if (!File.Exists(original)) throw new FileNotFoundException("The installed uninstaller was not found.", original);
            File.Copy(original, engine);
        }
        else
        {
            using (Stream payload = Assembly.GetExecutingAssembly().GetManifestResourceStream("Machinecoin.Payload"))
            {
                if (payload == null) throw new InvalidOperationException("Use the full setup executable to install Machinecoin.");
                using (var file = new FileStream(engine, FileMode.CreateNew, FileAccess.Write)) payload.CopyTo(file);
            }
        }
        // NSIS requires /D= and _?= to be the last argument, without quotation marks.
        string arguments = uninstall ? "/S _?=" + destination : "/S " + (test ? "/TEST=1 " : "") + "/D=" + destination;
        using (var process = Process.Start(new ProcessStartInfo(engine, arguments) { UseShellExecute = false, CreateNoWindow = true, WindowStyle = ProcessWindowStyle.Hidden }))
        {
            process.WaitForExit();
            int code = process.ExitCode;
            if (code == 0)
            {
                bool present = File.Exists(Path.Combine(destination, "machinecoin-qt.exe"));
                if (present == uninstall) code = 1;
            }
            File.WriteAllText(Path.Combine(work, "result.txt"), "Exit code: " + code);
            try { File.Delete(engine); } catch { }
            return code;
        }
    }

    private const string Layout = @"
<Window xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'
 Title='Machinecoin - Midnight Setup' Width='820' Height='650' MinWidth='700' MinHeight='610' WindowStartupLocation='CenterScreen'
 Background='#10151F' Foreground='#F5F6FA' FontFamily='Segoe UI' FontSize='14'>
 <Window.Resources>
  <Style TargetType='Button'>
   <Setter Property='Background' Value='#252E3D'/><Setter Property='Foreground' Value='#F5F6FA'/><Setter Property='BorderBrush' Value='#414B5D'/>
   <Setter Property='Padding' Value='22,12'/><Setter Property='Cursor' Value='Hand'/><Setter Property='FontWeight' Value='SemiBold'/>
   <Setter Property='Template'><Setter.Value><ControlTemplate TargetType='Button'>
    <Border x:Name='Surface' Background='{TemplateBinding Background}' BorderBrush='{TemplateBinding BorderBrush}' BorderThickness='1' CornerRadius='8' Padding='{TemplateBinding Padding}'>
     <ContentPresenter HorizontalAlignment='Center' VerticalAlignment='Center'/>
    </Border>
    <ControlTemplate.Triggers><Trigger Property='IsMouseOver' Value='True'><Setter TargetName='Surface' Property='Opacity' Value='0.85'/></Trigger>
     <Trigger Property='IsKeyboardFocused' Value='True'><Setter TargetName='Surface' Property='BorderBrush' Value='White'/><Setter TargetName='Surface' Property='BorderThickness' Value='2'/></Trigger>
     <Trigger Property='IsEnabled' Value='False'><Setter TargetName='Surface' Property='Opacity' Value='0.4'/></Trigger>
    </ControlTemplate.Triggers>
   </ControlTemplate></Setter.Value></Setter>
  </Style>
 </Window.Resources>
 <Grid Margin='38,30'>
  <Grid.RowDefinitions><RowDefinition Height='Auto'/><RowDefinition Height='Auto'/><RowDefinition Height='*'/><RowDefinition Height='Auto'/></Grid.RowDefinitions>
  <Grid><StackPanel Orientation='Horizontal'><Border Background='#F2B84B' Width='5' Height='38' CornerRadius='2' Margin='0,0,14,0'/>
   <StackPanel><TextBlock Text='MACHINECOIN' FontWeight='Bold' FontSize='17'/><TextBlock Text='MIDNIGHT WALLET' Foreground='#A7AEBB' FontSize='10' Margin='0,3,0,0'/></StackPanel></StackPanel>
   <TextBlock x:Name='Step' HorizontalAlignment='Right' VerticalAlignment='Center' FontSize='11' Foreground='#F2B84B'/></Grid>
  <StackPanel Grid.Row='1' Margin='0,32,0,22'><TextBlock x:Name='Heading' FontSize='30' FontWeight='SemiBold'/><TextBlock x:Name='Subtitle' Margin='0,10,0,0' TextWrapping='Wrap' Foreground='#A7AEBB' FontSize='15'/></StackPanel>
  <Border Grid.Row='2' CornerRadius='12' Background='#19212E' BorderBrush='#2B3545' BorderThickness='1' Padding='24' Margin='0,0,0,24'><Grid x:Name='Content'/></Border>
  <Grid Grid.Row='3'><TextBlock Text='LOCAL INSTALLATION  |  WINDOWS x64' FontSize='10' Foreground='#8993A4' VerticalAlignment='Center'/>
   <StackPanel Orientation='Horizontal' HorizontalAlignment='Right'><Button x:Name='Cancel' Content='Cancel' Margin='0,0,12,0'/><Button x:Name='Action' Content='Install wallet' Background='#F2B84B' BorderBrush='#F2B84B' Foreground='#171B23'/></StackPanel>
  </Grid>
 </Grid>
</Window>";
}
