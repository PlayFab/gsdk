package com.microsoft.azure.gaming;

/**
 * Contains shared code between JsonFileConfiguration and EnvironmentConfiguration
 */
abstract class ConfigurationBase implements Configuration {
    private final String titleId;
    private final String buildId;
    private final String region;

    protected ConfigurationBase()
    {
        // These are always set as environment variables, even with the new gsdk config json file
        this.titleId = System.getenv(TITLE_ID_VARIABLE_NAME);
        this.buildId = System.getenv(BUILD_ID_VARIABLE_NAME);
        this.region = System.getenv(REGION_VARIABLE_NAME);
    }

    @Override
    public String getTitleId() { return this.titleId; }

    @Override
    public String getBuildId() { return this.buildId; }

    @Override
    public String getRegion() { return this.region; }
}
