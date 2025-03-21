import fs from 'fs';
import path from 'path';
import { execSync } from 'child_process';

const zip = process.env['ZIP_BIN'] || "zip"
const script_dir = process.argv0.indexOf("node") > -1 ? path.dirname(process.argv[1]) : path.dirname(process.argv0);

function copyFiles(srcDir, destDir, files) {
	files.map(f => {
		fs.copyFileSync(path.join(srcDir, f), path.join(destDir, f));
	});
}

export default function makeDistro(platform = null, dirs = {}, buildInfo = null)
{
	buildInfo = buildInfo || JSON.parse(fs.readFileSync(path.join(script_dir, "version.json"), "utf8"));
	const root = dirs.root || path.resolve(script_dir, "../");
	const dist = dirs.dist || path.join(root, "dist");
	const src = dirs.src || path.join(root, "src");
	//const bins = dirs.bins || path.join(build, "bin");
	platform = platform || buildInfo.PLATFORM_OS || (process.platform === "win32" ? "Windows" : process.platform === "darwin" ? "MacOS" : "Linux");
	const build = dirs.build || path.join(dist, platform, buildInfo.SYSTEM_NAME);

	const packageName = path.join(dist, `${buildInfo.SYSTEM_NAME}-${platform}-${buildInfo.VERSION_STR.split(' ')[0]}.tpp`);
	var result;

	console.info("Generating entry.tp");
	result = execSync(`node ${path.resolve(script_dir, "./gen_entry.js")} -o ${build}`);
	console.info(String(result));

	console.info("Copying files to", build);
	copyFiles(root, build, [ 'README.md', 'CHANGELOG.md', 'LICENSE.txt' ]);
	copyFiles(path.join(src, "resources", "images"), build, [ 'tp_icon.png' ]);

	console.info("Creating archive", packageName);
	result =  execSync(`${zip} -FS -r ${packageName} . -x *.log`, { cwd: path.join(build, "..") });
	console.info(String(result));

	console.info("Finished!");
}

makeDistro();
