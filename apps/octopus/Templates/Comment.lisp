(Grid Columns="36px 1fr" Spacing=12 MarginLeft=(bind "DataContext/Indent")
  (Button Name="Avatar" Width=36 Height=36 Click="OpenAuthor" BackgroundColor="#00000000" class="octopus-row"
    (Grid Width=36 Height=36 StateClasses=(bind "DataContext/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Initials") FontSize=13 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/HasAvatar") Src=(bind "DataContext/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true)))
  (StackView Spacing=4
    (Grid Columns="1fr auto" Spacing=8 IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/AuthorName") FontSize=14 FontWeight="Bold" TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-ink" IgnoreHitTest=true)
      (TextBlock Text=(bind "DataContext/Age") FontSize=12 class="octopus-dim" IgnoreHitTest=true))
    (TextBlock Name="CommentBody" Text=(bind "DataContext/Body") FontSize=15 WordWrap=true class="octopus-ink" IgnoreHitTest=true)
    (StackView Direction="Horizontal" AlignItems="Center" Spacing=20 Height=30 StateClasses=(bind "DataContext/State")
      (Button Name="Like" Height=30 Width=64 Click="LikeComment" IgnoreHitTest=(bind "DataContext/Locked") BackgroundColor="#00000000" class="octopus-row reaction-button"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=6 IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/heart.svg" Width=16 Height=16 MarginTop="auto" MarginBottom="auto" class="reaction like-reaction" IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/LikesLabel") FontSize=13 class="reaction like-reaction" IgnoreHitTest=true)))
      (Button Name="Reply" Click="Reply" Height=30 Width=48 FontSize=13 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" BackgroundColor="#00000000" class="octopus-action octopus-link" "reply"))))
