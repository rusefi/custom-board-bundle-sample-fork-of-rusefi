package com.rusefi.tools.tune;

import com.opensr5.ini.IniFileModel;
import com.opensr5.ini.field.ArrayIniField;
import org.jetbrains.annotations.NotNull;

import java.io.*;
import java.util.Arrays;
import java.util.Date;

/**
 * tuner studio project to C data structure converter command line utility
 * <p>
 * 12/27/2014
 * Andrey Belomutskiy, (c) 2012-2016
 */
@SuppressWarnings("StringConcatenationInsideStringBufferAppend")
public class TS2C {
    public static String FINGER_PRINT = "/* Generated by " + TS2C.class.getSimpleName() + " on " + new Date() + "*/\n";

    /**
     * @see TS2CRunner
     */
    public static void main(String[] args) throws IOException {
        new TS2C(args);
    }

    private TS2C(String[] args) throws IOException {
        System.out.println("This tool reads TS tune file and produces some C code for hardcoded base tunes");
        if (args.length != 3 && args.length != 4 && args.length != 5) {
            System.out.println("Three parameters expected: ");
            System.out.println("  INPUT_MSQ_FILE NAME LOAD_SECTION_NAME RPM_SECTION_NAME [TABLE_NAME]");
            System.out.println("for example");
            // section names are needed in order to generate comments about cell content
            System.out.println("  currenttune.msq veLoadBins veRpmBins veTable");
            System.exit(-1);
        }
        String msqFileName = args[0];
        String loadSectionName = args[1];
        String rpmSectionName = args[2];
        String tableName = args.length == 3 ? "none" : args[3];

        IniFileModel model = IniFileModel.getInstance();

        String entityName = tableName.equalsIgnoreCase("none") ? loadSectionName : tableName;
        String methodName = getMethodName(entityName);

        BufferedWriter w = new BufferedWriter(new FileWriter("generated_" + methodName + ".cpp"));


        CurveData loadBins = null;
        CurveData rpmBins = null;

        if (!loadSectionName.equalsIgnoreCase("none")) {
            loadBins = CurveData.processCurve(msqFileName, loadSectionName, model, w);
        }

        if (!rpmSectionName.equalsIgnoreCase("none")) {
            rpmBins = CurveData.processCurve(msqFileName, rpmSectionName, model, w);
        }

        if (!tableName.equalsIgnoreCase("none")) {
            String sb = getTableCSourceCode2(msqFileName, tableName, model, rpmBins, loadBins);

            w.write(sb);

        }

        w.write(FINGER_PRINT);
        w.write("static void set" + methodName + "() {\n");
        w.write("\tMEMCPY(config->" + loadSectionName + ", hardCoded" + loadSectionName + ");\n");
        w.write("\tMEMCPY(config->" + rpmSectionName + ", hardCoded" + rpmSectionName + ");\n");

        if (!tableName.equalsIgnoreCase("none")) {
            w.write("\tMEMCPY(config->" + tableName + ", hardCoded" + tableName + ");\n");
        }


        w.write("}\n");

        w.close();
    }

    public static String getTableCSourceCode2(String msqFileName, String tableName, IniFileModel model) throws IOException {
        String xRpmBinsName = model.getXBin(tableName);
        String yLoadBinsName = model.getYBin(tableName);

        CurveData xRpmCurve = CurveData.valueOf(msqFileName, xRpmBinsName, model);
        CurveData yLoadCurve = CurveData.valueOf(msqFileName, yLoadBinsName, model);
        return getTableCSourceCode2(msqFileName, tableName, model, xRpmCurve, yLoadCurve);
    }

    @NotNull
    public static String getTableCSourceCode2(String msqFileName, String tableName, IniFileModel model, CurveData xRpmCurve, CurveData yLoadBins) throws IOException {
        float[][] table = readTable(msqFileName, tableName, model);

        return getTableCSourceCode(tableName, yLoadBins, xRpmCurve, table);
    }

    private static String getTableCSourceCode(String tableName, CurveData loadBins, CurveData rpmBins, float[][] table) {
        StringBuilder sb = new StringBuilder();

        sb.append("static const float hardCoded" + tableName + "[" + table.length + "][" + table[0].length + "] = {\n");

        writeTable(loadBins, rpmBins, sb, (loadIndex, rpmIndex) -> table[loadIndex][rpmIndex]);
        sb.append("};\n\n");
        return sb.toString();
    }

    @NotNull
    private static float[][] readTable(String msqFileName, String tableName, IniFileModel model) throws IOException {
        ArrayIniField field = (ArrayIniField) model.allIniFields.get(tableName);

        if (field.getRows() != field.getCols())
            throw new UnsupportedOperationException("Not square table not supported yet");
        // todo: replace with loadCount & rpmCount
        int size = field.getRows();

        float[][] table = new float[size][];
        for (int i = 0; i < size; i++) {
            table[i] = new float[size];
        }

        BufferedReader reader = readAndScroll(msqFileName, tableName);
        readTable(table, reader, size);
        return table;
    }

    @NotNull
    private static String getMethodName(String methodName) {
        return methodName.toUpperCase().charAt(0) + methodName.substring(1);
    }

    private static void writeTable(CurveData loadBins, CurveData rpmBins, StringBuilder sb, ValueSource valueSource) {
        sb.append(FINGER_PRINT);
        for (int loadIndex = 0; loadIndex < loadBins.getRawData().length; loadIndex++) {
            String line = writeTableLine(loadBins, rpmBins, valueSource, loadIndex);
            sb.append(line);
        }
    }

    /**
     * @param fileName       text file to open
     * @param magicStringKey magic string content to scroll to
     * @return Reader after the magicStringKey line
     */
    static BufferedReader readAndScroll(String fileName, String magicStringKey) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(fileName));
        System.out.println("Reading from " + fileName + ", scrolling to " + magicStringKey);
        String line;
        while ((line = reader.readLine()) != null) {
            if (line.contains(magicStringKey)) {
                System.out.println("Found " + line);
                break;
            }
        }
        return reader;
    }

    private static String writeTableLine(CurveData loadBins, CurveData rpmBins, ValueSource valueSource, int loadIndex) {
        StringBuilder sb = new StringBuilder("{");

        sb.append("/* " + loadIndex + " " + String.format("%3.3f", loadBins.getRawData()[loadIndex]) + "\t*/");
        for (int rpmIndex = 0; rpmIndex < rpmBins.getRawData().length; rpmIndex++) {
            sb.append("/* " + rpmIndex + " " + rpmBins.getRawData()[rpmIndex] + "*/" + String.format("%3.3f", valueSource.getValue(loadIndex, rpmIndex)) + ",\t");
        }
        sb.append("},\n");

        return sb.toString();
    }

    @NotNull
    public static String getCopyMethodBody(String tableReference, IniFileModel model, String tableName) {
        String xRpmBinsName = model.getXBin(tableName);
        String yLoadBinsName = model.getYBin(tableName);

        String x = "\tcopyArray(" + tableReference + "LoadBins, hardCoded" + xRpmBinsName + ");\n" +
                "\tcopyArray(" + tableReference + "RpmBins, hardCoded" + yLoadBinsName + ");\n" +
                "\tcopyTable(" + tableReference + "Table, hardCoded" + tableName + ");\n";
        return x;
    }

    interface ValueSource {
        float getValue(int loadIndex, int rpmIndex);
    }

    private static void readTable(float[][] table, BufferedReader r, int size) throws IOException {
        int index = 0;

        while (index < size) {
            String line = r.readLine();
            if (line == null)
                throw new IOException("End of file?");
            line = line.trim();
            if (line.isEmpty())
                continue;

            String[] values = line.split("\\s");
            if (values.length != size)
                throw new IllegalStateException("Expected " + size + " but got " + Arrays.toString(values) + ". Unexpected line: " + line);

            for (int i = 0; i < size; i++) {
                String str = values[i];
                try {
                    table[index][i] = Float.parseFloat(str);
                } catch (NumberFormatException e) {
                    throw new IllegalStateException("While reading " + str, e);
                }
            }
            System.out.println("Got line " + index + ": " + Arrays.toString(table[index]));
            index++;
        }
    }

}
