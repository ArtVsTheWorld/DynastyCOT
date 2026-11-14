using System;

namespace OfflineDynastySettings
{
    internal sealed class GameLaunch
    {
        internal int Pid;
        internal long Started;
        internal DateTime StartedUtc;
        internal bool Same(GameLaunch other) { return other != null && Pid == other.Pid && Started == other.Started; }
    }
    internal interface ILaunchEnvironment
    {
        DateTime Now { get; }
        GameLaunch FindGame();
        bool IsAlive(GameLaunch game);
        ILaunchSession Begin(GameLaunch game);
        void Log(string message);
    }
    internal interface ILaunchFailureDetails { void Detail(Exception error); }
    internal interface ILaunchSession : IDisposable
    {
        string InstallClock();
        bool InstallPlaycall();
        void Poll();
        void Verify();
    }
    internal sealed class HookOwnershipException : Exception
    { internal HookOwnershipException(string message) : base(message) { } }

    // One watcher lives for MMC's lifetime. Neither launch callback cancellation
    // nor the end of a previous match can consume a later launch notification.
    internal sealed class LaunchCoordinator : IDisposable
    {
        private readonly ILaunchEnvironment environment;
        private GameLaunch game;
        private ILaunchSession session;
        private bool clockReady, playcallReady, ownershipFailed;
        private DateTime nextAttempt, nextVerify;
        private string status;
        internal LaunchCoordinator(ILaunchEnvironment environment) { this.environment = environment; }
        private bool Report(string message)
        {
            if (message == status) return false;
            status = message;
            try { environment.Log(message); } catch { } // Logging must not disable activation.
            return true;
        }
        internal void Step()
        {
            try {
                GameLaunch found = environment.FindGame();
                // An incomplete process snapshot must not abandon our saved
                // handles and then mistake our own installed hooks for a conflict.
                if (found == null && game != null && environment.IsAlive(game)) found = game;
                if ((found == null && game != null) || (found != null && !found.Same(game))) {
                    DisposeSession(); game = found;
                    clockReady = playcallReady = ownershipFailed = false;
                    nextAttempt = nextVerify = DateTime.MinValue;
                    Report(found == null ? "Waiting for CFB 27; watcher remains ready for the next launch."
                        : "Detected CFB 27 PID " + found.Pid + "; checking this new game process.");
                }
                if (game == null) { Report("Waiting for CFB 27; watcher remains ready for the next launch."); return; }
                if (ownershipFailed) return;
                DateTime now = environment.Now;
                if (now - game.StartedUtc < TimeSpan.FromSeconds(45)) {
                    Report("Waiting for the game to finish initial startup (PID " + game.Pid + ")."); return;
                }
                if (now < nextAttempt) return;
                nextAttempt = now.AddSeconds(1);
                if (session == null) session = environment.Begin(game);
                if (!clockReady) {
                    string result = session.InstallClock();
                    if (!result.StartsWith("Installed", StringComparison.Ordinal) && result != "Already installed") {
                        Report("Play Clock not active yet (PID " + game.Pid + "): " + result + ". Will retry without overwriting incompatible code."); return;
                    }
                    clockReady = true;
                    Report("Play Clock installed in PID " + game.Pid + ". Checking playcall hooks and the settings helper.");
                }
                if (!playcallReady) {
                    if (!session.InstallPlaycall()) {
                        session.Poll();
                        Report("Playcall settings not active yet (PID " + game.Pid + "): waiting for compatible code and the verified Clock writer. Will retry."); return;
                    }
                    session.Verify(); nextVerify = now.AddSeconds(5);
                    playcallReady = true;
                    Report(ReadyMessage());
                }
                if (now >= nextVerify) { session.Verify(); nextVerify = now.AddSeconds(5); }
                session.Poll();
                Report(ReadyMessage());
                nextAttempt = now; // Save setting changes every watcher tick after activation.
            } catch (HookOwnershipException ex) {
                ownershipFailed = true;
                Report("Activation stopped for this game: " + ex.Message + " Unknown hooks will not be overwritten. Restart the game after resolving the conflict.");
            } catch (Exception ex) {
                nextAttempt = environment.Now.AddSeconds(1);
                if(Report(ex is UnauthorizedAccessException
                    ?"Permission denied while preparing Extra Settings for PID "+(game==null?"unknown":game.Pid.ToString())+". Check that MMC can read its runtime and write its AppData settings. "+ex.Message
                    :"Will retry for this game: "+ex.Message)){
                    var details=environment as ILaunchFailureDetails;
                    if(details!=null)try{details.Detail(ex);}catch{}
                }
            }
        }
        private string readyStatus;
        private string ReadyMessage(){
            if(readyStatus==null)readyStatus="ACTIVE: Play Clock and playcall hooks verified in PID "+game.Pid+". The settings helper is ready. Match-only changes wait for an eligible offline Dynasty game.";
            return readyStatus;
        }
        private void DisposeSession()
        { readyStatus=null;if (session != null) { try { session.Dispose(); } catch { } session = null; } }
        public void Dispose() { DisposeSession(); }
    }
}
