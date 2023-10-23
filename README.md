# behavior_rknn
# 第一次上传
git init
git remote add origin git@github.com:negan1123/behavior_rknn.git

git add README.md
git commit -m "first commit"
git branch -M main
git push origin main

# 后续上传
git remote add origin git@github.com:negan1123/behavior_rknn.git
git branch -M main
git push -u origin main

# 上传到已有分支
git pull origin 远程分支名
git checkout 远程分支名
git add 文件
git push origin 远程分支名