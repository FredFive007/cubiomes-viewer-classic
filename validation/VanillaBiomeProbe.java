import net.fabricmc.api.ModInitializer;
import net.fabricmc.fabric.api.event.lifecycle.v1.ServerLifecycleEvents;
import net.minecraft.core.registries.Registries;
import net.minecraft.world.level.biome.BiomeManager;
import net.minecraft.world.level.levelgen.NoiseBasedChunkGenerator;
import net.minecraft.world.level.levelgen.RandomState;
import java.nio.file.*;
import java.io.*;
import java.util.Random;

// Run in a disposable 26.3 Fabric server with only Fabric API and this probe.
public class VanillaBiomeProbe implements ModInitializer {
    public void onInitialize() {
        ServerLifecycleEvents.SERVER_STARTED.register(server -> {
            try (var out = Files.newBufferedWriter(Path.of("vanilla-26.3-biomes.csv"))) {
                out.write("seed,scale,x,y,z,biome\n");
                var gen = (NoiseBasedChunkGenerator) server.overworld().getChunkSource().getGenerator();
                var noises = server.registryAccess().lookupOrThrow(Registries.NOISE);
                long[] seeds = {-3829811542736183482L, 0L, 1L, -1L, 12345L, Long.MIN_VALUE, Long.MAX_VALUE, 987654321L};
                for (long seed : seeds) {
                    var state = RandomState.create(noises, seed, gen.generatorSettings().value());
                    var resolver = gen.getBiomeSource().createUncachedResolver(state);
                    var manager = new BiomeManager(resolver, BiomeManager.obfuscateSeed(seed));
                    var random = new Random(263);
                    for (int i = 0; i < 256; ++i) {
                        int x = random.nextInt(200001) - 100000;
                        int z = random.nextInt(200001) - 100000;
                        int y = new int[]{-48, -16, 32, 77}[i % 4];
                        if (i == 0) { x = 68148; y = 77; z = 80990; }
                        String block = manager.getBiome(x, y, z).unwrapKey().orElseThrow().identifier().getPath();
                        String quart = resolver.getNoiseBiome(x >> 2, y >> 2, z >> 2).unwrapKey().orElseThrow().identifier().getPath();
                        out.write(seed + ",1," + x + "," + y + "," + z + "," + block + "\n");
                        out.write(seed + ",4," + (x >> 2) + "," + (y >> 2) + "," + (z >> 2) + "," + quart + "\n");
                    }
                }
                System.out.println("[VanillaBiomeProbe] COMPLETE 4096 samples");
            } catch (Throwable e) {
                e.printStackTrace();
            } finally {
                server.halt(false);
            }
        });
    }
}
