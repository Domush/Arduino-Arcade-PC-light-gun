import fs from 'fs';
import path from 'path';
import { execSync } from 'child_process';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const rootDir = path.resolve(__dirname, '..');

const devHtmlPath = path.join(rootDir, 'index.dev.html');
const indexHtmlPath = path.join(rootDir, 'index.html');

console.log('🔄 Preparing development template for Vite build...');
if (fs.existsSync(devHtmlPath)) {
  fs.copyFileSync(devHtmlPath, indexHtmlPath);
}

console.log('🚀 Building website with Vite...');
execSync('npm run build', { cwd: rootDir, stdio: 'inherit' });

const distDir = path.join(rootDir, 'dist');
if (!fs.existsSync(distDir)) {
  console.error('❌ dist directory not found!');
  process.exit(1);
}

console.log('📦 Staging compiled production assets for GitHub Pages root...');

// Copy compiled production index.html to root
fs.copyFileSync(path.join(distDir, 'index.html'), indexHtmlPath);

// Copy assets directory to root
const distAssets = path.join(distDir, 'assets');
const rootAssets = path.join(rootDir, 'assets');
if (fs.existsSync(distAssets)) {
  if (!fs.existsSync(rootAssets)) {
    fs.mkdirSync(rootAssets, { recursive: true });
  }

  // Clean obsolete assets in root
  const existingFiles = fs.readdirSync(rootAssets);
  for (const f of existingFiles) {
    fs.unlinkSync(path.join(rootAssets, f));
  }

  // Copy fresh compiled assets
  const files = fs.readdirSync(distAssets);
  for (const f of files) {
    fs.copyFileSync(path.join(distAssets, f), path.join(rootAssets, f));
  }
}

// Ensure .nojekyll exists so GitHub Pages does not ignore assets
fs.writeFileSync(path.join(rootDir, '.nojekyll'), '');

console.log('✅ Production bundle ready at root.');

try {
  console.log('📤 Committing and pushing to origin/gh-pages...');
  execSync('git add -A', { cwd: rootDir, stdio: 'inherit' });
  
  try {
    execSync('git commit -m "deploy: initial publication of Arduino Arcade PC Light Gun website on gh-pages"', { cwd: rootDir, stdio: 'inherit' });
  } catch (commitErr) {
    console.log('ℹ️ No new changes to commit.');
  }

  execSync('git push -u origin gh-pages', { cwd: rootDir, stdio: 'inherit' });
  console.log('\n🎉 Successfully deployed website to GitHub Pages!');
  console.log('🌐 Live site URL: https://domush.github.io/Arduino-Arcade-PC-light-gun/\n');
} catch (err) {
  console.error('⚠️ Git push error:', err.message);
  process.exit(1);
}
