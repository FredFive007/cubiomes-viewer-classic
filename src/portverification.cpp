// Regression checks for the independent 26.3 port. Uses isolated settings.
#include "mainwindow.h"
#include "conditiondialog.h"
#include "searchthread.h"
#include "util.h"
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QSet>
#include <QThread>
#include <QTextStream>

int runPortVerification(int argc, char **argv, const QString& output)
{
    QDir().mkpath(output);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, output + "/settings");
    QApplication app(argc, argv);
    QFile report(output + "/verification.txt");
    if (!report.open(QIODevice::WriteOnly | QIODevice::Text)) return 2;
    QTextStream log(&report);
    int failures = 0;
    auto check = [&](bool ok, const QString& name) {
        log << (ok ? "PASS " : "FAIL ") << name << '\n';
        log.flush();
        if (!ok) ++failures;
    };
    constexpr uint64_t seed = uint64_t(-3829811542736183482LL);
    check(str2mc("26.3") == MC_26_3, "version parser");
    check(QString(mc2str(MC_26_3)) == "26.3", "version serialization");
    Generator g;
    setupGenerator(&g, MC_26_3, 0);
    applySeed(&g, DIM_OVERWORLD, seed);
    check(getBiomeAt(&g, 1, 68148, 77, 80990) == dappled_forest,
          "upstream 26.3 dappled forest reference coordinate");
    check(biomeExists(MC_26_3, sulfur_caves) && biomeExists(MC_26_3, dappled_forest),
          "new biome registry");

    auto evaluate = [&](Condition c, uint64_t s) {
        ConditionTree tree;
        if (!tree.set({c}, MC_26_3).isEmpty()) return -1;
        SearchThreadEnv env;
        if (!env.init(MC_26_3, false, &tree).isEmpty()) return -2;
        std::atomic_bool stop(false);
        env.setSeed(s);
        Pos path[100] = {};
        return testTreeAt({0, 0}, &env, PASS_FULL_64, &stop, path);
    };
    Condition forest = {};
    forest.version = Condition::VER_CURRENT;
    forest.type = F_BIOME;
    forest.save = 1;
    forest.count = 1;
    forest.step = 1;
    forest.x1 = forest.x2 = 68148;
    forest.z1 = forest.z2 = 80990;
    forest.y = 77;
    forest.biomeToFindM = 1ULL << (dappled_forest - 128);
    check(evaluate(forest, seed) == COND_OK, "Viewer biome condition accepts reference seed");
    Condition wrongBiome = forest;
    wrongBiome.biomeToFindM = 1ULL << (sulfur_caves - 128);
    check(evaluate(wrongBiome, seed) == COND_FAILED, "Viewer biome condition rejects incorrect biome");
    Condition sulfur = wrongBiome;
    sulfur.x1 = sulfur.x2 = 8540;
    sulfur.z1 = sulfur.z2 = 10386;
    sulfur.y = -16;
    check(evaluate(sulfur, seed) == COND_OK, "Viewer sulfur caves filter accepts vanilla reference point");
    sulfur.biomeToFindM = 0;
    sulfur.biomeToExclM = 1ULL << (sulfur_caves - 128);
    check(evaluate(sulfur, seed) == COND_FAILED, "Viewer sulfur caves exclusion rejects reference point");
    Condition restored = {};
    check(restored.readHex(forest.toHex()) && evaluate(restored, seed) == COND_OK,
          "new biome filter save/load round trip");

    // Compare actual Viewer filtering with direct generation across a fixed seed batch.
    int matches = 0, mismatches = 0;
    QSet<QString> expectedSeeds;
    for (uint64_t s = seed; s < seed + 64; ++s) {
        applySeed(&g, DIM_OVERWORLD, s);
        bool expected = getBiomeAt(&g, 1, forest.x1, forest.y, forest.z1) == dappled_forest;
        bool found = evaluate(forest, s) == COND_OK;
        matches += found;
        mismatches += found != expected;
        if (expected) expectedSeeds.insert(QString::number(qint64(s)));
    }
    check(mismatches == 0 && matches > 0 && matches < 64,
          QString("64-seed filter batch vs direct engine (%1 matches)").arg(matches));

    Session session;
    session.wi.mc = MC_26_3;
    session.cv = {forest};
    session.gen48.mode = GEN48_NONE;
    session.sc.searchtype = SEARCH_INC;
    session.sc.startseed = session.sc.smin = seed;
    session.sc.smax = seed + 63;
    session.sc.stoponres = false;
    session.sc.threads = 2;
    QFile sessionFile(output + "/batch.save");
    check(session.save(nullptr, sessionFile.fileName(), true), "save classic search session");
    QProcess batch;
    batch.start(QCoreApplication::applicationFilePath(),
        {"--nogui", "--session", sessionFile.fileName(), "--out", output + "/batch-results.txt"});
    bool finished = batch.waitForFinished(20000);
    if (!finished) { batch.kill(); batch.waitForFinished(); }
    QFile batchLog(output + "/batch.log");
    if (batchLog.open(QIODevice::WriteOnly)) {
        batchLog.write(batch.readAllStandardOutput());
        batchLog.write(batch.readAllStandardError());
    }
    QFile results(output + "/batch-results.txt");
    QSet<QString> actualSeeds;
    if (results.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!results.atEnd()) {
            QString line = QString::fromUtf8(results.readLine()).trimmed();
            if (!line.isEmpty() && !line.startsWith('#')) actualSeeds.insert(line);
        }
    }
    check(finished && batch.exitStatus() == QProcess::NormalExit && batch.exitCode() == 0
          && actualSeeds == expectedSeeds, "actual two-thread headless search returns expected seed set");
    applySeed(&g, DIM_OVERWORLD, seed);
    StructureConfig sc = {};
    Pos campPos = {};
    check(getStructureConfig(Abandoned_Camp, MC_26_3, &sc), "abandoned camp configuration callback");
    bool campFound = false;
    for (int rz = -4; rz <= 4 && !campFound; ++rz)
        for (int rx = -4; rx <= 4 && !campFound; ++rx)
            if (getStructurePos(Abandoned_Camp, MC_26_3, seed, rx, rz, &campPos)
                && isViableStructurePos(Abandoned_Camp, &g, campPos.x, campPos.z, 0))
                campFound = true;
    check(campFound, "find viable abandoned camp");
    Condition camp = {};
    camp.version = Condition::VER_CURRENT;
    camp.type = F_CAMP;
    camp.save = 1;
    camp.count = 1;
    camp.x1 = camp.x2 = campPos.x;
    camp.z1 = camp.z2 = campPos.z;
    check(campFound && evaluate(camp, seed) == COND_OK, "Viewer camp condition accepts generated camp");
    camp.count = 2;
    check(evaluate(camp, seed) == COND_FAILED, "Viewer camp count rejects second camp at same point");

    // The old Viewer used length-delimited quad tables; the new engine uses a
    // zero terminator. Exercise a known quad-monument base through the Viewer.
    StructureConfig monumentConfig;
    getStructureConfig(Monument, MC_26_3, &monumentConfig);
    Condition quad = {};
    quad.version = Condition::VER_CURRENT;
    quad.type = F_QM_90;
    quad.save = 1;
    quad.count = 1;
    quad.x1 = quad.z1 = 0;
    quad.x2 = quad.z2 = 1024;
    uint64_t quadSeed = (35634735275ULL - monumentConfig.salt) & MASK48;
    check(evaluate(quad, quadSeed) == COND_OK, "classic quad-monument search with new engine table API");

    QSettings settings(APP_STRING, APP_STRING);
    settings.setValue("config/lang", "zh_CN");
    settings.setValue("config/checkForUpdates", false);
    settings.sync();
    {
        MainWindow window(output + "/smoke-session.save", "");
        window.setAttribute(Qt::WA_DontShowOnScreen);
        WorldInfo wi;
        wi.mc = MC_26_3;
        wi.seed = seed;
        wi.y = 77;
        wi.large = false;
        window.setSeed(wi, DIM_OVERWORLD);
        window.mapGoto(68148, 80990, 2);
        window.formCond->addItemCondition(nullptr, forest);
        window.resize(1440, 900);
        window.show();
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < 2500) {
            app.processEvents();
            QThread::msleep(20);
        }
        auto *versions = window.findChild<QComboBox*>("comboBoxMC");
        check(versions && versions->findText("26.3") >= 0 && versions->currentText() == "26.3",
              "GUI selects 26.3");
        check(window.saction[D_CAMP] && !window.saction[D_CAMP]->icon().isNull(), "camp map icon loaded");
        check(window.grab().save(output + "/viewer-26.3.png"), "rendered main window snapshot");
        ConditionDialog dialog(window.formCond, window.mapView, &window.config, wi, nullptr, &forest);
        dialog.setAttribute(Qt::WA_DontShowOnScreen);
        dialog.show();
        app.processEvents();
        check(dialog.grab().save(output + "/biome-filter-26.3.png"), "rendered biome filter snapshot");
        dialog.hide();
        window.hide();
    }
    log << "Failures: " << failures << '\n';
    return failures ? 1 : 0;
}
