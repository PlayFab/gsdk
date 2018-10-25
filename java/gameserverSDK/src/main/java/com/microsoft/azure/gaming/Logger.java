package com.microsoft.azure.gaming;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;

import static java.nio.file.StandardOpenOption.*;

/**
 * Class that provides a simple logging interface.
 */
class Logger {
    private static String logFolder = null;
    private Path logFile = null;

    protected Logger()
    {
        String timestamp = DateTimeFormatter.ofPattern("yyyyMMdd_HHmmss").format(ZonedDateTime.now(ZoneOffset.UTC));
        this.logFile = Paths.get("GSDK_output_" + timestamp + ".txt");

        // If the logFolder is valid, append it to the logFile path
        // otherwise we'll default to current directory
        if (logFolder != null && !logFolder.isEmpty())
        {
            try
            {
                Path logPath = Paths.get(logFolder);
                Files.createDirectories(logPath);
                this.logFile = logPath.resolve(this.logFile);
            }
            catch(IOException | InvalidPathException e)
            {
                e.printStackTrace();
                System.out.println(e);
                System.out.println("Using current directory as log folder instead.");
            }
        }

        this.logFile = this.logFile.toAbsolutePath();
        System.out.println("Logging GSDK output to: " + this.logFile);
    }

    protected static void SetLogFolder(String folder) { logFolder = folder; }

    protected static Logger Instance()
    {
        return SingletonHelper.INSTANCE;
    }

    protected void Log(String message)
    {
        try(BufferedOutputStream bos = new BufferedOutputStream(
                Files.newOutputStream(this.logFile, CREATE, APPEND));
            PrintWriter out = new PrintWriter(bos))
        {
            out.println(message);
        }
        catch (IOException e)
        {
            System.err.println(e);
        }
    }

    protected void LogError(String message)
    {
        this.Log("ERROR: " + message);
    }

    protected void LogWarning(String message){
        this.Log("WARNING: " + message);
    }

    protected void LogError(Exception ex)
    {
        try(BufferedOutputStream bos = new BufferedOutputStream(
            Files.newOutputStream(this.logFile, CREATE, APPEND));
            PrintWriter out = new PrintWriter(bos))
        {
            ex.printStackTrace(out);
        }
        catch (IOException e)
        {
            System.err.println(e);
        }
    }

    /**
     * Using an inner static helper class to have a thread-safe
     * singleton.
     * https://www.journaldev.com/1377/java-singleton-design-pattern-best-practices-examples
     */
    private static class SingletonHelper
    {
        private static final Logger INSTANCE;

        static {
            INSTANCE = new Logger();
        }
    }
}
