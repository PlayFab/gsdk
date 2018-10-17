# Use OpenJDK as a base image
FROM openjdk:8

# Copy the executable
COPY *.jar /app/

# Set the working directory to /app
WORKDIR /app

# Run our game when the container launches
CMD ["java", "-jar", "javaTestRunnerGame.jar"]