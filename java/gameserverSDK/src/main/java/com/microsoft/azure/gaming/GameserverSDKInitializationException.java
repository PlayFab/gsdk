package com.microsoft.azure.gaming;

/**
 * Exception thrown when there was an issue initializing the GSDK.
 * Since it derives from RuntimeException, this is an unchecked exception,
 * because if the GSDK fails to initialize, that is a non-recoverable error.
 */
public class GameserverSDKInitializationException extends RuntimeException {
    public GameserverSDKInitializationException(String message){
        super(message);
    }

    public GameserverSDKInitializationException(String message, Throwable cause){
        super(message, cause);
    }
}
